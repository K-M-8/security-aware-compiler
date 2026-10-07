#include "lexer.h"
#include <cctype>
#include <stdexcept>
using namespace std;

static TokenType keyword(const string& s) {
    if (s == "secret") return TokenType::SECRET;
    if (s == "public") return TokenType::PUBLIC;
    if (s == "int") return TokenType::INT;
    if (s == "if") return TokenType::IF;
    if (s == "else") return TokenType::ELSE;
    if (s == "print") return TokenType::PRINT;
    return TokenType::IDENT;
}

vector<Token> Lexer::tokenize() {
    vector<Token> out;
    while (pos < source.size()) {
        char c = source[pos];
        if (c == ' ' || c == '\t' || c == '\r') { ++pos; ++column; continue; }
        if (c == '\n') { ++pos; ++line; column = 1; continue; }
        int startCol = column;
        if (isalpha(static_cast<unsigned char>(c)) || c == '_') {
            string s;
            while (pos < source.size() && (isalnum(static_cast<unsigned char>(source[pos])) || source[pos] == '_')) {
                s += source[pos++]; ++column;
            }
            out.push_back({keyword(s), s, line, startCol});
            continue;
        }
        if (isdigit(static_cast<unsigned char>(c))) {
            string s;
            while (pos < source.size() && isdigit(static_cast<unsigned char>(source[pos]))) {
                s += source[pos++]; ++column;
            }
            out.push_back({TokenType::NUMBER, s, line, startCol});
            continue;
        }
        auto add = [&](TokenType t, int n = 1) {
            string s = source.substr(pos, n);
            out.push_back({t, s, line, startCol});
            pos += n; column += n;
        };
        switch (c) {
            case '+': add(TokenType::PLUS); break;
            case '-': add(TokenType::MINUS); break;
            case '*': add(TokenType::STAR); break;
            case '/': add(TokenType::SLASH); break;
            case '(': add(TokenType::LPAREN); break;
            case ')': add(TokenType::RPAREN); break;
            case '{': add(TokenType::LBRACE); break;
            case '}': add(TokenType::RBRACE); break;
            case ';': add(TokenType::SEMICOLON); break;
            case ',': add(TokenType::COMMA); break;
            case '=': if (pos + 1 < source.size() && source[pos+1] == '=') add(TokenType::EQEQ, 2); else add(TokenType::EQ); break;
            case '!': if (pos + 1 < source.size() && source[pos+1] == '=') add(TokenType::NEQ, 2); else throw runtime_error("Unexpected ! at line " + to_string(line)); break;
            case '<': if (pos + 1 < source.size() && source[pos+1] == '=') add(TokenType::LE, 2); else add(TokenType::LT); break;
            case '>': if (pos + 1 < source.size() && source[pos+1] == '=') add(TokenType::GE, 2); else add(TokenType::GT); break;
            default: throw runtime_error("Unexpected character '" + string(1,c) + "' at line " + to_string(line));
        }
    }
    out.push_back({TokenType::END, "", line, column});
    return out;
}
