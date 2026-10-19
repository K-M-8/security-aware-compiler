from flask import Flask, render_template, request, jsonify
from pathlib import Path
import subprocess
import shutil
import os
import re

app = Flask(__name__)
BASE_DIR = Path(__file__).resolve().parent
OUTPUT_DIR = BASE_DIR / "output"
EXAMPLES_DIR = BASE_DIR / "examples"
COMPILER_EXE = BASE_DIR / "security_compiler.exe"
CUSTOM_FILE = BASE_DIR / "dashboard_custom_input.sac"

def find_examples():
    if not EXAMPLES_DIR.exists():
        return []
    return sorted(p.name for p in EXAMPLES_DIR.glob("*.sac") if p.is_file())

def ensure_compiler():
    if COMPILER_EXE.exists():
        return True, "Using existing security_compiler.exe."
    if shutil.which("g++") is None:
        return False, "g++ was not found. Install/use the same MinGW/MSYS2 compiler that you used to build the project, then retry."
    src_dir = BASE_DIR / "src"
    cpp_files = sorted(src_dir.glob("*.cpp")) if src_dir.exists() else []
    if not cpp_files:
        return False, "Could not find src/*.cpp. Extract the dashboard files into the main project folder, beside src/, examples/, and CMakeLists.txt."
    cmd = ["g++", "-std=c++17", *[str(p) for p in cpp_files], "-o", str(COMPILER_EXE)]
    result = subprocess.run(cmd, cwd=BASE_DIR, capture_output=True, text=True, timeout=90)
    if result.returncode != 0:
        details = (result.stderr or result.stdout or "Compiler build failed.").strip()
        return False, "Could not build the C++ engine:\n" + details
    return True, "Built security_compiler.exe from src/*.cpp."

def read_output_file(name):
    path = OUTPUT_DIR / name
    try:
        return path.read_text(encoding="utf-8", errors="replace") if path.exists() else ""
    except OSError:
        return ""

@app.get("/")
def index():
    return render_template("index.html", examples=find_examples())

@app.get("/api/examples")
def examples_api():
    return jsonify({"examples": find_examples(), "projectFolder": str(BASE_DIR)})

@app.get("/api/source/<name>")
def source_api(name):
    if Path(name).name != name or not name.endswith(".sac"):
        return jsonify({"error": "Invalid example filename"}), 400
    path = EXAMPLES_DIR / name
    if not path.is_file():
        return jsonify({"error": "Example not found"}), 404
    return jsonify({"name": name, "source": path.read_text(encoding="utf-8", errors="replace")})

@app.post("/api/analyze")
def analyze():
    data = request.get_json(silent=True) or {}
    selected = (data.get("example") or "").strip()
    source_text = data.get("source") or ""
    custom_mode = bool(data.get("customMode"))

    if custom_mode:
        if not source_text.strip():
            return jsonify({"ok": False, "error": "Paste a .sac program first."}), 400
        # Keep the custom input constrained to this project folder.
        CUSTOM_FILE.write_text(source_text, encoding="utf-8")
        input_path = CUSTOM_FILE
    else:
        if not selected or Path(selected).name != selected or not selected.endswith(".sac"):
            return jsonify({"ok": False, "error": "Choose a valid example file."}), 400
        input_path = EXAMPLES_DIR / selected
        if not input_path.exists():
            return jsonify({"ok": False, "error": "That example file was not found in examples/."}), 404

    ok, setup_message = ensure_compiler()
    if not ok:
        return jsonify({"ok": False, "error": setup_message}), 500

    try:
        result = subprocess.run(
            [str(COMPILER_EXE), str(input_path.relative_to(BASE_DIR))],
            cwd=BASE_DIR,
            capture_output=True,
            text=True,
            timeout=30
        )
        stdout = result.stdout or ""
        stderr = result.stderr or ""
        report = read_output_file("security_report.txt")
        tac = read_output_file("tac.txt")
        transformed = read_output_file("transformed_code.sac")
        combined = stdout + (("\n" + stderr) if stderr else "")
        # Derive headline counts from actual engine output; never fabricate scores.
        findings = len(re.findall(r"Finding\s*#?\s*\d+|Finding\s+\d+|HIGH|MEDIUM|LOW", combined, re.I))
        secret_refs = len(re.findall(r"\bSECRET\b", combined, re.I))
        repaired = bool(transformed.strip()) and ("ct_select" in transformed or "ct_select" in combined)
        has_error = result.returncode != 0 or bool(re.search(r"\b(error|failed)\b", combined, re.I))
        return jsonify({
            "ok": result.returncode == 0,
            "returnCode": result.returncode,
            "setup": setup_message,
            "command": f".\\security_compiler.exe {input_path.relative_to(BASE_DIR)}",
            "stdout": stdout,
            "stderr": stderr,
            "report": report,
            "tac": tac,
            "transformed": transformed,
            "source": input_path.read_text(encoding="utf-8", errors="replace"),
            "example": input_path.name,
            "metrics": {
                "findingMentions": findings,
                "secretMentions": secret_refs,
                "repairGenerated": repaired
            },
            "status": "Completed" if result.returncode == 0 else "Engine returned a non-zero exit code"
        })
    except subprocess.TimeoutExpired:
        return jsonify({"ok": False, "error": "Analysis timed out after 30 seconds. Check the C++ engine for a stuck process."}), 504
    except Exception as exc:
        return jsonify({"ok": False, "error": str(exc)}), 500
    finally:
        if custom_mode:
            try:
                CUSTOM_FILE.unlink(missing_ok=True)
            except OSError:
                pass

@app.get("/api/output/<name>")
def output_api(name):
    allowed = {"security_report.txt", "tac.txt", "transformed_code.sac"}
    if name not in allowed:
        return jsonify({"error": "File not allowed"}), 400
    return jsonify({"name": name, "content": read_output_file(name)})

if __name__ == "__main__":
    print("Security-Aware Compiler Dashboard")
    print(f"Project folder: {BASE_DIR}")
    print("Open http://127.0.0.1:5000 in your browser.")
    app.run(host="127.0.0.1", port=5000, debug=False)
