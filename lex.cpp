#include "lex.h"
#include <string>
#include <assert.h>
#include <map>
#include <algorithm>
#include <sstream>
#include <vector>

#define EOI (-1)


namespace GSP {

    struct Token : public IToken {
        Token();

        Token(TokenType tp, const std::string &word) : type_(tp), word_(word), word1_(word) {}

        Token(TokenType tp, const std::string &word, const std::string &word1) : type_(tp), word_(word),
                                                                                 word1_(word1) {}

        virtual TokenType type() const override { return type_; }

        virtual const char *word() const override { return word_.c_str(); }

        virtual const char *word_semantic() const override { return word1_.c_str(); }

        void set(TokenType tp, const std::string &word) { type_ = tp; word_ = word; word1_ = word; }
        void set(TokenType tp, const std::string &word, const std::string &word1) { type_ = tp; word_ = word; word1_ = word1; }

        TokenType type_;
        std::string word_;  // for lex
        std::string word1_; // for semantic
    };

    struct Lex : public ILex {
        Lex(const char *sql);

        virtual IToken *token() override { return &cur_tk_; }

        virtual ILex *clone() override { return new Lex(*this); }

        virtual void recover(ILex *state) override {
            auto other = dynamic_cast<Lex*>(state);
            if (other == nullptr) return;
            this->cur_tk_ = other->cur_tk_;
            this->sql_ = other->sql_;

            this->pos_ = other->pos_;
            this->line_ = other->line_;
            this->col_ = other->col_;
            //this->keyword_ = other->keyword_;
            //this->keyword1_ = other->keyword1_;
        }

        virtual void next() override { scanf(); }

        virtual unsigned int cur_pos_line() const override { return line_; }

        virtual unsigned int cur_pos_col() const override { return col_; }

        virtual unsigned int cur_pos() const override { return pos_; }

        virtual const std::string &get_token_type_name(TokenType token_type) const override {
            //auto f = keyword1_.find(token_type);
            //assert(f != keyword1_.end());
            //return f->second;
            return "";
        }


        void scanf();

        void white();

        void comment();

        void single_line_comment();

        void multi_line_comment();

        void scanf_operator() {
            switch (char_at(pos())) {
                case '^': {
                    cur_tk_.set(CARET, "^");
                    pos_inc(1);
                }
                    break;
                case '%': {
                    cur_tk_.set(PERCENT, "%");
                    pos_inc(1);
                }
                    break;
                case '+': {
                    cur_tk_.set(PLUS, "+");
                    pos_inc(1);
                }
                    break;
                case '-': {
                    cur_tk_.set(MINUS, "-");
                    pos_inc(1);
                }
                    break;
                case '*': {
                    cur_tk_.set(STAR, "*");
                    pos_inc(1);
                }
                    break;
                case '/': {
                    cur_tk_.set(DIVIDE, "/");
                    pos_inc(1);
                }
                    break;
                case '.': {
                    cur_tk_.set(DOT, ".");
                    pos_inc(1);
                }
                    break;
                case '(': {
                    cur_tk_.set(LPAREN, "(");
                    pos_inc(1);
                }
                    break;
                case ')': {
                    cur_tk_.set(RPAREN, ")");
                    pos_inc(1);
                }
                    break;
                case ',': {
                    cur_tk_.set(COMMA, ",");
                    pos_inc(1);
                }
                    break;
                case ';': {
                    cur_tk_.set(SEMI, ";");
                    pos_inc(1);
                }
                    break;
                case '?': {
                    cur_tk_.set(QUES, "?");
                    pos_inc(1);
                }
                    break;
                case '=': {
                    cur_tk_.set(EQ, "=");
                    pos_inc(1);
                }
                    break;
                case '!': {
                    pos_inc(1);
                    if (char_at(pos()) == '=') {
                        cur_tk_.set(LTGT, "!=");
                        pos_inc(1);
                    } else {
                        cur_tk_.set(ERR, "EXPECTED '!='");
                    }
                }
                    break;
                case '>': {
                    pos_inc(1);
                    if (char_at(pos()) == '=') {
                        cur_tk_.set(GTEQ, ">=");
                        pos_inc(1);
                    } else {
                        cur_tk_.set(GT, ">");
                    }

                }
                    break;
                case '<': {
                    pos_inc(1);
                    if (char_at(pos()) == '=') {
                        cur_tk_.set(LTEQ, "<=");
                        pos_inc(1);
                    } else if (char_at(pos()) == '>') {
                        cur_tk_.set(LTGT, "<>");
                        pos_inc(1);
                    } else {
                        cur_tk_.set(LT, "<");
                    }
                }
                    break;
                case '|': {
                    pos_inc(1);
                    if (char_at(pos()) == '|') {
                        cur_tk_.set(BARBAR, "||");
                        pos_inc(1);
                    } else {
                        cur_tk_.set(ERR, "EXPECTED '||'");
                    }
                }
                    break;
                default: {
                    std::string err_s = "UNEXPECTED ";
                    cur_tk_.set(ERR, err_s + char_at(pos()));
                }
                    break;
            }
        }

        void scanf_str_literal() {
            unsigned int start = pos();
            assert(char_at(pos()) == '\'');
            pos_inc(1);
            char c = char_at(pos());
            std::stringstream buf{};
            while (c != EOI) {
                if (c == '\'') {
                    if (char_at(pos() + 1) == '\'') {
                        buf << '\'';
                        pos_inc(1);
                    } else
                        break;
                } else
                    buf << c;
                c = char_at(pos_inc(1));
            }
            if (c == '\'') {
                pos_inc(1);
                cur_tk_.set(STR_LITERAL, sub(start, pos()), buf.str());
            } else
                cur_tk_.set(ERR, "UNTERMINATED STRING LITERAL");
        }

        void scanf_identifier() {
            unsigned int start = pos();
            char c = char_at(pos());
            switch (c) {
                case '"': {
                    pos_inc(1);
                    c = char_at(pos());
                    std::stringstream buf{};
                    while (c != EOI) {
                        if (c == '"') {
                            if (char_at(pos() + 1) == '"') {
                                buf << '"';
                                pos_inc(1);
                            } else
                                break;
                        } else
                            buf << c;
                        c = char_at(pos_inc(1));
                    }
                    if (c == '"') {
                        pos_inc(1);
                        cur_tk_.set(ID, sub(start, pos()), buf.str());
                    } else
                        cur_tk_.set(ERR, "IDENTIFIER WITH UNTERMINATED \"");
                }
                    break;
                case '[': {
                    pos_inc(1);
                    c = char_at(pos());
                    std::stringstream buf{};
                    while (c != EOI) {
                        if (c == ']') {
                            if (char_at(pos() + 1) == ']') {
                                buf << ']';
                                pos_inc(1);
                            } else
                                break;
                        } else
                            buf << c;
                        c = char_at(pos_inc(1));
                    }
                    if (c == ']') {
                        pos_inc(1);
                        cur_tk_.set(ID, sub(start, pos()), buf.str());
                    } else
                        cur_tk_.set(ERR, "IDENTIFIER WITH UNTERMINATED [");
                }
                    break;
                default: {
                    if (is_identifier_begin(c)) {
                        pos_inc(1);
                        c = char_at(pos());
                        while (is_identifier_body(c)) {
                            c = char_at(pos_inc(1));
                        }

                        std::string s = sub(start, pos());
                        if (!check_reserved_keyword(s))
                            cur_tk_.set(ID, s);
                    } else {
                        std::string err_s = "UNEXPECTED ";
                        cur_tk_.set(ERR, err_s + c);
                    }

                }
                    break;
            }
        }

        void scanf_number() {
            unsigned int start = pos();
            char c = char_at(pos());
            assert(is_dec_body(c) || c == '.');
            if (c == '.') {
                /* [0-9]+ */
                c = char_at(pos_inc(1));
                if (is_dec_body(c)) {
                    while (is_dec_body(c))
                        c = char_at(pos_inc(1));
                } else {
                    cur_tk_.set(ERR, "ERR NUMBER .");
                    return;
                }
            } else {
                pos_inc(1);
                c = char_at(pos());
                if ('X' == c || 'x' == c) {
                    /* Hexadecimal Number */
                    c = char_at(pos_inc(1));
                    while (is_hex_body(c))
                        c = char_at(pos_inc(1));
                    cur_tk_.set(NUMBER, sub(start, pos()));
                    return;
                } else {
                    while (is_dec_body(c))
                        c = char_at(pos_inc(1));
                    if (c == '.') {
                        c = char_at(pos_inc(1));
                        /* [0-9]* */
                        while (is_dec_body(c))
                            c = char_at(pos_inc(1));
                    }
                }

            }
            if (c == 'e' || c == 'E') {
                c = char_at(pos_inc(1));
                if (c == '-' || c == '+')
                    c = char_at(pos_inc(1));
                if (is_dec_body(c)) {
                    c = char_at(pos_inc(1));
                    while (is_dec_body(c))
                        c = char_at(pos_inc(1));
                } else {
                    cur_tk_.set(ERR, "ERR NUMBER E");
                    return;
                }
            }
            cur_tk_.set(NUMBER, sub(start, pos()));
        }

        bool is_dec_body(char c) {
            return '0' <= c && c <= '9';
        }

        bool is_hex_body(char c) {
            return ('0' <= c && c <= '9') ||
                   ('A' <= c && c <= 'F') ||
                   ('a' <= c && c <= 'f');
        }

        bool is_identifier_begin(char c) {
            return ('a' <= c && c <= 'z') ||
                   ('A' <= c && c <= 'Z') ||
                   '_' == c;
        }

        bool is_identifier_body(char c) {
            return is_identifier_begin(c) || ('0' <= c && c <= '9');
        }

        bool is_white(char c) {
            return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f';
        }

        unsigned pos_inc(unsigned int inc) {
            pos_ += inc;
            col_ += inc;
            if (pos_ > sql_.length())
                cur_tk_.set(END_P, "");
            return pos_;
        }

        unsigned pos() const { return pos_; }

        void new_line() {
            line_++;
            col_ = 0;
        }

        /* todo */
#if 0
        bool check_reserved_keyword(const std::string &word) { return false; }
#else
        bool check_reserved_keyword(const std::string &word) {
            std::string big = word;
            std::transform(big.begin(), big.end(), big.begin(), ::toupper);
            auto it = keyword_.find(big);
            if (it != keyword_.end()) {
                cur_tk_.set(it->second, word);
                return true;
            }
            return false;
        }
#endif

        char char_at(unsigned int pos) {
            if (pos < sql_.length())
                return sql_[pos];
            else
                return EOI;
        }

        std::string sub(unsigned int start, unsigned int end) {
            return sql_.substr(start, end - start);
        }

        Token cur_tk_;
        std::string sql_;

        unsigned int pos_;
        unsigned int line_;
        unsigned int col_;
        static const std::map<std::string, TokenType> keyword_;
        static std::map<TokenType, std::string> keyword1_;
        //std::vector<TokenType> hash_; /* todo */
    };

    Token::Token() : type_(none) {}


/* todo */
const std::map<std::string, TokenType> Lex::    keyword_{
            {"ALL",       ALL},
            {"AND",       AND},
            {"ANY",       ANY},
            {"AS",        AS},
            {"ASC",       ASC},
            {"AVG",       AVG},
            {"BETWEEN",   BETWEEN},
            {"BY",        BY},
            {"CASE",      CASE},
            {"CAST",      CAST},
            {"COLLATE",   COLLATE},
            {"CONVERT",   CONVERT},
            {"COUNT",     COUNT},
            {"CROSS",     CROSS},
            {"DEFAULT",   DEFAULT},
            {"DELETE",    DELETE},
            {"DESC",      DESC},
            {"DISTINCT",  DISTINCT},
            {"ELSE",      ELSE},
            {"END",       END},
            {"ESCAPE",    ESCAPE},
            {"EXCEPT",    EXCEPT},
            {"EXISTS",    EXISTS},
            {"FALSE",     FALSE},
            {"FROM",      FROM},
            {"FULL",      FULL},
            {"GROUP",     GROUP},
            {"GROUPING",  GROUPING},
            {"HAVING",    HAVING},
            {"IN",        IN},
            {"INNER",     INNER},
            {"INSERT",    INSERT},
            {"INTERSECT", INTERSECT},
            {"INTERVAL",  INTERVAL},
            {"INTO",      INTO},
            {"IS",        IS},
            {"JOIN",      JOIN},
            {"LEFT",      LEFT},
            {"LIKE",      LIKE},
            {"MAX",       MAX},
            {"MATCH",     MATCH},
            {"MIN",       MIN},
            {"NATURAL",   NATURAL},
            {"NOT",       NOT},
            {"NULLIF",    NULLIF},
            {"NULL",      NULLX},
            {"ON",        ON},
            {"OR",        OR},
            {"ORDER",     ORDER},
            {"OUTER",     OUTER},
            {"OVER",      OVER},
            {"OVERLAPS",  OVERLAPS},
            {"PARTIAL",   PARTIAL},
            {"RANK",      RANK},
            {"RIGHT",     RIGHT},
            {"SELECT",    SELECT},
            {"SET",       SET},
            {"SOME",      SOME},
            {"SUM",       SUM},
            {"THEN",      THEN},
            {"TO",        TO},
            {"TRUE",      TRUE},
            {"UNION",     UNION},
            {"UNIQUE",    UNIQUE},
            {"UNKNOWN",   UNKNOWN},
            {"UPDATE",    UPDATE},
            {"VALUES",    VALUES},
            {"WHEN",      WHEN},
            {"WHERE",     WHERE},
            {"WITH",      WITH}
    };
    Lex::Lex(const char *sql) : sql_(sql), pos_(0), line_(0), col_(0)
    {

    }

    void Lex::scanf() {
        if (cur_tk_.type_ == ERR || cur_tk_.type_ == END_P) return;
        while (true) {
            switch (char_at(pos())) {
                case ' ':
                case '\t':
                case '\r':
                case '\n':
                case '\f': {
                    white();
                }
                    break;
                case '-': {
                    if (char_at(pos() + 1) == '-')
                        comment();
                    else
                        return scanf_operator();
                }
                    break;
                case '/': {
                    if (char_at(pos() + 1) == '*')
                        comment();
                    else
                        return scanf_operator();
                }
                    break;
                case '^':
                case '%':
                case '*':
                case '+':
                case '.':
                case '(':
                case ')':
                case ',':
                case ';':
                case '?':
                case '=':
                case '>':
                case '<':
                case '!':
                case '|': {
                    if ('.' == char_at(pos()) && is_dec_body(char_at(pos() + 1)))
                        return scanf_number();
                    return scanf_operator();
                }
                    break;
                case '\'': {
                    return scanf_str_literal();
                }
                    break;
                case '"':
                case '[': {
                    return scanf_identifier();
                }
                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9': {
                    return scanf_number();
                }
                    break;
                case EOI: {
                    cur_tk_.set(END_P, "");
                    return;
                }
                    break;
                default: {
                    if (is_identifier_begin(char_at(pos()))) {
                        return scanf_identifier();
                    } else {
                        char c = char_at(pos());
                        std::string err_s = "UNEXPECTED ";
                        cur_tk_.set(ERR, err_s + c);
                        return;
                    }
                }
                    break;
            }
        }
    }

    void Lex::white() {
        char c = EOI;
        while (is_white(c = char_at(pos()))) {
            pos_inc(1);
            if (c == '\n')
                new_line();
        }

    }

    void Lex::comment() {
        if (char_at(pos()) == '-' && char_at(pos() + 1) == '-') {
            single_line_comment();
        } else if (char_at(pos()) == '/' && char_at(pos() + 1) == '*') {
            multi_line_comment();
        }
    }

    void Lex::single_line_comment() {
        pos_inc(2);    /* skip -- */
        while (char_at(pos()) != EOI && char_at(pos()) != '\n')
            pos_inc(1);
        if (char_at(pos()) == '\n') {
            pos_inc(1);
            new_line();
        }

    }

    void Lex::multi_line_comment() {
        pos_inc(2);  /* skip */
        char c1 = char_at(pos()), c2 = char_at(pos() + 1);
        while (c1 != EOI && c2 != EOI) {
            if (c1 == '*' && c2 == '/')
                break;
            pos_inc(1);
            c1 = char_at(pos()), c2 = char_at(pos() + 1);
        }
        if (c1 == '*' && c2 == '/')
            pos_inc(2);  /* skip  */
        else {
            /* unterminated multiline comment */
            cur_tk_.set(ERR, "unterminated multiline comment");
        }
    }

    ILex *make_lex(const char *sql) {
        return new Lex(sql);
    }

    void free_lex(ILex *lex) {
        delete (lex);
    }

}