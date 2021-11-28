#include "./LuaLexer.hpp"

/*
    rust source code

use crate::code::Code;
    use crate::lexer::Lexer;
    use crate::token::{Token, TokenType};

    fn tokenize(contents: &str) -> Vec<Token> {
        let code = Code::new(contents, "test");
        let mut lexer = Lexer::new(code);

        let (tokens, errors) = lexer.get_tokens();

        for error in &errors {
            println!("{}", error);
        }

        tokens
    }

    fn one_token(tokens: Vec<Token>) -> Token {
        if tokens.len() != 2 {
            panic!("expected 1 token, got {}", tokens.len());
        }

        assert_eq!(tokens[1].kind, TokenType::EndOfFile);

        tokens[0].clone()
    }

    fn tokens_to_string(tokens: Vec<Token>) -> String {
        let mut result = String::new();

        for token in &tokens {
            for whitespace_token in &token.whitespace {
                result.push_str(&whitespace_token.value);
            }
            result.push_str(&token.value);
        }

        result
    }

    fn expect_error(contents: &str, expected_error: &str) -> Vec<Token> {
        let code = Code::new(contents, "test");
        let mut lexer = Lexer::new(code);

        let (tokens, errors) = lexer.get_tokens();

        for error in &errors {
            if error.message.contains(expected_error) {
                return tokens;
            }
        }

        println!("could not find error {} got these instead:", expected_error);

        for error in &errors {
            println!("\t{}", error);
        }

        panic!("expected error, got no errors");
    }

    fn check(code: &str) {
        let actual = tokens_to_string(tokenize(code));

        assert_eq!(actual, code);
    }

    #[test]
    fn tokens_to_string_test() {
        check("local foo =   5 + 2..2");
    }

    #[test]
    fn smoke() {
        assert_eq!(tokenize("")[0].kind, TokenType::EndOfFile);
        assert_eq!(one_token(tokenize("a")).kind, TokenType::Letter);
        assert_eq!(one_token(tokenize("1")).kind, TokenType::Number);
        assert_eq!(one_token(tokenize("(")).kind, TokenType::Symbol);
    }

    #[test]
    fn shebang() {
        assert_eq!(tokenize("#!/usr/bin/env lua")[0].kind, TokenType::Shebang);
    }

    #[test]
    fn single_quote_string() {
        assert_eq!(one_token(tokenize("'1'")).kind, TokenType::String);
    }

    #[test]
    fn z_escaped_string() {
        assert_eq!(one_token(tokenize("\"a\\z\na\"")).kind, TokenType::String);
    }

    #[test]
    fn number_range() {
        assert_eq!(tokenize("1..20").len(), 4);
    }
    #[test]
    fn number_delimiter() {
        assert_eq!(tokenize("1_000_000").len(), 2);
        assert_eq!(tokenize("0xdead_beef").len(), 2);
        assert_eq!(tokenize("0b0101_0101").len(), 2);
    }

    #[test]
    fn number_annotations() {
        assert_eq!(tokenize("50ull").len(), 2);
        assert_eq!(tokenize("50uLL").len(), 2);
        assert_eq!(tokenize("50ULL").len(), 2);
        assert_eq!(tokenize("50LL").len(), 2);
        assert_eq!(tokenize("50lL").len(), 2);
        assert_eq!(tokenize("1.5e+20").len(), 2);
        assert_eq!(tokenize(".0").len(), 2);
    }

    #[test]
    fn malformed_number() {
        expect_error("12LOL", "malformed decimal number");
        expect_error("0xbLOL", "malformed hex number");
        expect_error("0b101LOL01", "malformed binary number");
        expect_error("1.5eD", "after 'exponent'");
        expect_error("1.5e+D", "malformed exponent expected number, got D");
    }

    #[test]
    fn multiline_comment_error() {
        expect_error("/*", "tried to find end of multiline c comment");
        expect_error("--[[", "unclosed multiline comment");
    }

    #[test]
    fn string_error() {
        expect_error("\"woo\nfoo", "expected quote to end");
        expect_error("'aaa", "expected quote to end: reached end of file");
    }

    #[test]
    fn multiline_string() {
        assert_eq!(tokenize("a = [[a]]").len(), 3);
        assert_eq!(tokenize("a = [=[a]=]").len(), 3);
        assert_eq!(tokenize("a = [==[a]==]").len(), 3);

        expect_error("a = [=a", "malformed multiline string");
        expect_error("a = [[a", "expected multiline string reached end of code");
    }

    #[test]
    fn multiline_comment() {
        assert_eq!(tokenize("--[[a]]")[0].kind, TokenType::EndOfFile);
        assert_eq!(tokenize("--[=[a]=]")[0].kind, TokenType::EndOfFile);
        assert_eq!(tokenize("--[==[a]==]")[0].kind, TokenType::EndOfFile);
        assert_eq!(tokenize("/*a* /")[0].kind, TokenType::EndOfFile);
    }

    #[test]
    fn line_comment() {
        assert_eq!(tokenize("-- a")[0].kind, TokenType::EndOfFile);
        assert_eq!(tokenize("// a")[0].kind, TokenType::EndOfFile);
        assert_eq!(tokenize("--[= a")[0].kind, TokenType::EndOfFile);
    }

    #[test]
    fn comment_escape() {
        assert_eq!(one_token(tokenize("--[[# 1337 ]]")).kind, TokenType::Number);
    }

    #[test]
    fn typesystem_symbols() {
        assert_eq!(tokenize("$'foo'").len(), 3);
    }
    #[test]
    fn unknown_symbols() {
        assert_eq!(tokenize("```").len(), 4);
    }

    #[test]
    fn debug_code() {
        assert_eq!(one_token(tokenize("§foo = true")).kind, TokenType::AnalyzerDebugCode);
        assert_eq!(one_token(tokenize("£foo = true")).kind, TokenType::ParserDebugCode);
    }

*/

vector<Token *> Tokenize(const string &code)
{
    auto code_obj = new Code(code, "test");
    LuaLexer lexer(code_obj);
    auto [tokens, errors] = lexer.GetTokens();

    for (auto &error : errors)
    {
        printf("%s", error.what());
    }

    return tokens;
}

void RunTokenizerTests()
{
    auto tokens = Tokenize("local foo = 1");
}