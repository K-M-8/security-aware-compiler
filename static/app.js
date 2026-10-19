const $ = (id) => document.getElementById(id);
let latest = null;
let activeResult = "report";

const resultMap = {
  report: () => latest?.report || latest?.stdout || "No security report was generated.",
  tac: () => latest?.tac || "No TAC output was generated.",
  repair: () => latest?.transformed || "No transformed_code.sac was generated for this run.",
  source: () => latest?.source || "No source loaded."
};

function toast(message) {
  const el = $("toast");
  el.textContent = message;
  el.classList.add("show");
  setTimeout(() => el.classList.remove("show"), 3500);
}
function setConsole(text, isError=false) {
  $("console").textContent = text;
  if (isError) $("console").style.color = "#ff9e9e";
  else $("console").style.color = "";
  $("console").scrollTop = $("console").scrollHeight;
}
function renderResult(tab=activeResult) {
  activeResult = tab;
  document.querySelectorAll(".result-tab").forEach(b => b.classList.toggle("active", b.dataset.result === tab));
  $("resultOutput").textContent = resultMap[tab]();
}
function countChars() {
  $("sourceCount").textContent = `${$("sourceInput").value.length} characters`;
}
async function refreshExamples() {
  try {
    const r = await fetch("/api/examples");
    const data = await r.json();
    const select = $("exampleSelect");
    const previous = select.value;
    select.innerHTML = "";
    data.examples.forEach(name => {
      const option = document.createElement("option");
      option.value = name; option.textContent = name; select.appendChild(option);
    });
    if (data.examples.includes(previous)) select.value = previous;
    if (!data.examples.length) toast("No .sac files found in examples/. Check the project folder.");
  } catch { toast("Could not load example list."); }
}
async function loadExample() {
  const name = $("exampleSelect").value;
  if (!name) return toast("No example files were found.");
  // The example content is fetched through a safe analyze-free endpoint below.
  try {
    const r = await fetch(`/api/source/${encodeURIComponent(name)}`);
    const data = await r.json();
    if (!r.ok) throw new Error(data.error || "Could not load example.");
    $("sourceInput").value = data.source;
    countChars();
    toast(`Loaded ${name}`);
  } catch (e) {
    toast(e.message);
  }
}
async function analyze() {
  const customText = $("sourceInput").value.trim();
  const customMode = customText.length > 0;
  if (!customMode && !$("exampleSelect").value) return toast("Choose an example or paste a program first.");
  const btn = $("analyzeBtn");
  btn.disabled = true;
  btn.innerHTML = "<span>◌</span> Analyzing… <b>⏳</b>";
  $("terminalLive").textContent = "RUNNING";
  $("terminalLive").style.color = "#f7c66a";
  $("engineState").textContent = "Executing C++ engine…";
  setConsole("$ Initializing security-aware compiler…\n$ Running lexical, parsing and security-analysis stages…");
  $("statusMetric").textContent = "Running";
  try {
    const response = await fetch("/api/analyze", {
      method: "POST", headers: {"Content-Type": "application/json"},
      body: JSON.stringify({example: $("exampleSelect").value, source: customText, customMode})
    });
    const data = await response.json();
    latest = data;
    if (!response.ok || !data.ok) {
      const error = data.error || data.stderr || data.stdout || "Analysis failed.";
      setConsole("$ ERROR\n" + error, true);
      $("statusMetric").textContent = "Error";
      $("statusSub").textContent = "See execution console";
      $("engineState").textContent = "Needs attention";
      $("returnCode").textContent = `EXIT ${data.returnCode ?? "—"}`;
      $("resultName").textContent = data.example || "ANALYSIS ERROR";
      renderResult("report");
      toast("Analysis did not complete. Check the console.");
      return;
    }
    const consoleText = [
      "$ " + data.command,
      "",
      "✓ " + data.setup,
      "",
      data.stdout || "(Engine completed with no standard output.)",
      data.stderr ? "\nSTDERR:\n" + data.stderr : "",
      "",
      `Process exit code: ${data.returnCode}`
    ].join("\n");
    setConsole(consoleText);
    $("statusMetric").textContent = "Complete";
    $("statusSub").textContent = data.status;
    $("findingMetric").textContent = data.metrics.findingMentions;
    $("secretMetric").textContent = data.metrics.secretMentions;
    $("repairMetric").textContent = data.metrics.repairGenerated ? "Generated" : "None";
    $("engineState").textContent = "Analysis process completed";
    $("returnCode").textContent = `EXIT ${data.returnCode}`;
    $("terminalLive").textContent = "COMPLETE";
    $("terminalLive").style.color = "#7be0a4";
    $("resultName").textContent = (data.example || "CUSTOM INPUT").toUpperCase();
    renderResult(activeResult);
    toast("Analysis finished. Review the report and TAC tabs.");
  } catch (e) {
    setConsole("$ CONNECTION ERROR\n" + e.message, true);
    $("statusMetric").textContent = "Error";
    $("statusSub").textContent = "Dashboard request failed";
    $("engineState").textContent = "Request failed";
    toast("Could not communicate with the dashboard backend.");
  } finally {
    btn.disabled = false;
    btn.innerHTML = "<span>▶</span> Analyze program <b>→</b>";
    if ($("terminalLive").textContent === "RUNNING") $("terminalLive").textContent = "IDLE";
  }
}

$("sourceInput").addEventListener("input", countChars);
$("loadExample").addEventListener("click", loadExample);
$("analyzeBtn").addEventListener("click", analyze);
$("refreshExamples").addEventListener("click", refreshExamples);
$("clearConsole").addEventListener("click", () => setConsole("$ Console cleared.\nReady for the next run."));
document.querySelectorAll(".result-tab").forEach(b => b.addEventListener("click", () => renderResult(b.dataset.result)));
document.querySelectorAll(".nav-item").forEach(b => b.addEventListener("click", () => {
  document.querySelectorAll(".nav-item").forEach(x => x.classList.toggle("active", x === b));
  const target = b.dataset.tab;
  if (target === "ir") renderResult("tac");
  if (target === "repair") renderResult("repair");
  if (target === "analyzer") window.scrollTo({top: 0, behavior: "smooth"});
  if (target !== "analyzer") document.querySelector(".results-panel").scrollIntoView({behavior: "smooth", block: "start"});
}));
refreshExamples();
countChars();
