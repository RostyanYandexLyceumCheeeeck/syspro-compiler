#include <headers/Lexer.hpp>

#include <optional>
#include <algorithm>
#include <string_view>
#include <unordered_map>

#include <ctre.hpp>


namespace {

template <size_t N>
struct FixedString {
    char buf[N];
    constexpr FixedString(const char (&s)[N]) { std::copy_n(s, N, buf); }

    constexpr explicit operator bool() const { return N > 1 && buf[0] != '\0'; }
    constexpr operator std::string_view() const { return {buf, N > 0 ? N - 1 : 0}; }
};


template <FixedString Pattern>
std::optional<std::string_view> matchPattern(std::string_view s) {
    auto match = ctre::starts_with<Pattern.buf>(s.begin(), s.end());
    
    if (match) { return match.to_view(); }
    return std::nullopt;
}


using PointerFunc = std::optional<std::string_view> (*)(std::string_view);

struct LexerRule {
    TokenType type;
    PointerFunc matcher;
    std::string_view errorValue = "";
};


const std::unordered_map<std::string_view, TokenType> KEYWORDS = {
    {"return",   TokenType::RETURN},
    {"val",      TokenType::VAL},
    {"var",      TokenType::VAR},
    {"if",       TokenType::IF},
    {"else",     TokenType::ELSE},
    {"while",    TokenType::WHILE},
    {"break",    TokenType::BREAK},
    {"continue", TokenType::CONTINUE},
    {"true",     TokenType::TRUE},
    {"false",    TokenType::FALSE},
    {"def",      TokenType::DEF},
    {"extern",   TokenType::EXTERN},
    {"Int8",     TokenType::INT8},
    {"Int16",    TokenType::INT16},
    {"Int32",    TokenType::INT32},
    {"Int64",    TokenType::INT64},
    {"Bool",     TokenType::BOOL},
    {"String",   TokenType::STRING},
    {"cast",     TokenType::CAST},
    {"struct",   TokenType::STRUCT},
};

constexpr LexerRule RULES[] = {
    // spaces, comments, errors(comments) and strings
    { TokenType::SPACE,       matchPattern<R"(\s+)">                                               },
    { TokenType::SINGLE_COMM, matchPattern<R"(//[^\n]*)">                                          },
    { TokenType::MULTI_COMM,  matchPattern<R"(/\*[\s\S]*?\*/)">                                    },
    { TokenType::ERROR,       matchPattern<R"(/\*[\s\S]*)">,     "Unterminated multi-line comment" },
    { TokenType::STR,         matchPattern<R"("([^"\\]|\\.)*")">                                   },
    { TokenType::STR,         matchPattern<R"('([^'\\]|\\.)*')">                                   },

    // number and ident
    { TokenType::INT,         matchPattern<R"(0|[1-9]\d*)">   },
    { TokenType::IDENT,       matchPattern<R"([a-zA-Z_]\w*)"> },

    // brackets
    { TokenType::LPAREN,      matchPattern<R"(\()"> },
    { TokenType::RPAREN,      matchPattern<R"(\))"> },
    { TokenType::LBRACE,      matchPattern<R"(\{)"> },
    { TokenType::RBRACE,      matchPattern<R"(\})"> },
    { TokenType::LBRACKET,    matchPattern<R"(\[)"> },
    { TokenType::RBRACKET,    matchPattern<R"(\])"> },

    // operatitons
    { TokenType::AND,         matchPattern<R"(&&)">   },
    { TokenType::OR,          matchPattern<R"(\|\|)"> },
    { TokenType::LE,          matchPattern<R"(<=)">   },
    { TokenType::GE,          matchPattern<R"(>=)">   },
    { TokenType::NE,          matchPattern<R"(!=)">   },
    { TokenType::EQ,          matchPattern<R"(==)">   },

    // operatitons
    { TokenType::ASSIGN,      matchPattern<R"(=)">   },
    { TokenType::LT,          matchPattern<R"(<)">   },
    { TokenType::GT,          matchPattern<R"(>)">   },
    { TokenType::NOT,         matchPattern<R"(!)">   },
    { TokenType::PLUS,        matchPattern<R"(\+)">  },
    { TokenType::MINUS,       matchPattern<R"(-)">   },
    { TokenType::MULT,        matchPattern<R"(\*)">  },
    { TokenType::DIV,         matchPattern<R"(/)">   },

    // dot, colon, comma and semicolon
    { TokenType::DOT,         matchPattern<R"(\.)">  },
    { TokenType::COLON,       matchPattern<R"(:)">   },
    { TokenType::COMMA,       matchPattern<R"(,)">   },
    { TokenType::SEMI,        matchPattern<R"(;)">   },
};

void updatePos(std::string_view sv, size_t& offset, size_t& line, size_t& column) {
    offset += sv.length();
    column += sv.length();

    auto last_n = sv.rfind('\n');
    if (last_n != std::string_view::npos) {
        line += std::count(sv.begin(), sv.end(), '\n');
        column = sv.length() - last_n;
    }
}

} // namespace


std::vector<Token> Lexer::parse(std::string_view src) const {
    std::vector<Token> tokens;

    size_t offset = 0, line = 1, column = 1;
    while (offset < src.size()) {
        std::string_view remaining = src.substr(offset);

        bool matched = false;
        for (const auto& rule : RULES) {
            
            auto obj = rule.matcher(remaining);
            if (obj) {
                std::string_view sv = obj.value();
                TokenType actualType = rule.type;

                if (actualType == TokenType::IDENT) {
                    auto it = KEYWORDS.find(sv);
                    if (it != KEYWORDS.end()) { actualType = it->second; }
                }

                std::string tokenValue = rule.errorValue.empty() ? std::string(sv) : std::string(rule.errorValue);

                tokens.emplace_back(actualType, std::move(tokenValue), line, column);
                updatePos(sv, offset, line, column);
                matched = true;
                break;
            }
        }

        if (!matched) {
            tokens.emplace_back(TokenType::ERROR, std::string(remaining.substr(0, 1)), line, column);
            offset++;
            column++;
        }
    }

    tokens.emplace_back(TokenType::END, "", line, column);
    return tokens;
}