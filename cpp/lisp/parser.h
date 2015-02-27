#ifndef _MONSTER_AVENGERS_MONSTER_DATA_PARSER_
#define _MONSTER_AVENGERS_MONSTER_DATA_PARSER_

#include <cstdio>
#include <cwchar>
#include <memory>
#include <string>
#include <locale>
#include <iosfwd>
#include <sstream>
#include <fstream>
#include <iostream>
#include <type_traits>

#include "lisp_object.h"
#include "helpers.h"

namespace monster_avengers {
  
  namespace parser {
    
    enum TokenName {
      OPEN_PARENTHESIS = 0, // (
      CLOSE_PARENTHESIS, // )
      KEYWORD, // :keyword
      STRING, // "string"
      TRUE, // T
      NUMBER, // 1234
      NIL, // NIL
      INVALID_TOKEN, 
      NUMBER_OF_TOKEN, // number of valid tokens
    };

    struct Token {
      TokenName name;
      std::wstring value;

      void Acquire(Token &&other) {
	name = other.name;
	value = std::move(other.value);
      }

      void DebugPrint() {
        wprintf(L"Token: ");
        switch (name) {
        case OPEN_PARENTHESIS:
          wprintf(L"(\n");
          break;
        case CLOSE_PARENTHESIS:
          wprintf(L")\n");
          break;
        case KEYWORD:
          wprintf(L":%ls\n", value.c_str());
          break;
        case STRING:
          wprintf(L"\"%Ls\"\n", value.c_str());
          break;
        case TRUE:
          wprintf(L"T\n");
          break;
        case NUMBER:
          wprintf(L"%ls\n", value.c_str());
          break;
        case NIL:
          wprintf(L"NIL\n");
          break;
        default:
          wprintf(L"INVALID_TOKEN\n");
        };
      }
    };

    class Tokenizer {
    public:

      // Move Constructor
      Tokenizer(Tokenizer &&other) {
        input_stream_ = std::move(other.input_stream_);
        buffer_ = other.buffer_;
        end_of_file_ = other.end_of_file_;
      }

      // Move Assignment
      const Tokenizer &operator=(Tokenizer &&other) {
        input_stream_ = std::move(other.input_stream_);
        buffer_ = other.buffer_;
        end_of_file_ = other.end_of_file_;
        return *this;
      }

      // Tokenizer from file.
      static Tokenizer FromFile(const std::string &file_name) {
        std::wifstream *input_stream = new std::wifstream(file_name);
        if (!input_stream->good()) {
          Log(ERROR, L"error while opening %s.", file_name.c_str());
          exit(-1);
        }
        return Tokenizer(input_stream);
      }

      // Tokenizer from wstring.
      static Tokenizer FromText(std::wstring text) {
        std::wistringstream *input_stream = new std::wistringstream(text);
        return Tokenizer(input_stream);
      }
      
      // Returns false if end of file. Returns true otherwise, even if
      // the read token may be invalid.
      bool Next(Token *token) {
        while (!end_of_file_ && std::isspace(buffer_)) GetChar();
        if (!end_of_file_) {
          if (!(Read<OPEN_PARENTHESIS>(token) ||
                Read<CLOSE_PARENTHESIS>(token) ||
                Read<KEYWORD>(token) ||
                Read<STRING>(token) ||
                Read<TRUE>(token) ||
                Read<NUMBER>(token) ||
                Read<NIL>(token))) {
            token->name = INVALID_TOKEN;
          }
          return true;
        }
        return false;
      }

      bool Expect(TokenName name) {
        Token token;
        if (Next(&token) && token.name == name) {
          return true;
        }
        return false;
      }
      
      TokenName ExpectOpenParenOrNil() {
        Token token;
        CHECK(Next(&token));
        CHECK(NIL == token.name || OPEN_PARENTHESIS == token.name);
        return token.name;
      }
      
      int ExpectNumber() {
        Token token;
        CHECK(Next(&token) && NUMBER == token.name);
        return std::stoi(token.value);
      }

      std::wstring ExpectString() {
        Token token;
        CHECK(Next(&token) && STRING == token.name);
        return token.value;
      }
      
    private:
      template <TokenName T, TokenName H>
      using Enabler = typename std::enable_if<(T==H), void>::type;

      // Will obtain the ownership of input_stream.
      explicit Tokenizer(std::wistream *input_stream) 
        : input_stream_(input_stream), buffer_(), end_of_file_(false) {
        input_stream_->imbue(std::locale("en_US.UTF-8"));
        GetChar();
      }
      
      inline wchar_t GetChar() {
        if (!input_stream_->get(buffer_)) {
          end_of_file_ = true;
          buffer_ = L' ';
        }
        return buffer_;
      }

      template <TokenName Name>
      bool Read(Token *result,
                Enabler<Name, OPEN_PARENTHESIS> *unused = nullptr) {
        if (L'(' == buffer_) {
          GetChar();
          result->name = OPEN_PARENTHESIS;
          result->value = L"";
          return true;
        }
        return false;
      }

      template <TokenName Name>
      bool Read(Token *result,
                Enabler<Name, CLOSE_PARENTHESIS> *unused = nullptr) {
        if (L')' == buffer_) {
          GetChar();
          result->name = CLOSE_PARENTHESIS;
          result->value = L"";
          return true;
        }
        
        return false;
      }

      template <TokenName Name>
      bool Read(Token *result,
                Enabler<Name, KEYWORD> *unused = nullptr) {
        if (L':' == buffer_) {
          result->name = KEYWORD;
          result->value = L"";
          while (!std::isspace(GetChar())) {
            result->value += buffer_;
          }
          return true;
        }
        
        return false;
      }

      template <TokenName Name>
      bool Read(Token *result,
                Enabler<Name, STRING> *unused = nullptr) {
        if (L'"' == buffer_) {
          result->name = STRING;
          result->value = L"";
          bool escape = false;
          while (!end_of_file_) {
            if ('"' == GetChar() && !escape) {
              break;
            }
            if (L'\\' == buffer_) {
              escape = true;
            } else {
              escape = false;
              if ('"' == buffer_) {
                result->value += '|';
              } else {
                result->value += buffer_;
              }
            }
          }
          if (L'"' != buffer_) {
            Log(ERROR, L"Invalid String.");
          }
          GetChar();
          return true;
        }
        
        return false;
      }

      template <TokenName Name>
      bool Read(Token *result,
                Enabler<Name, TRUE> *unused = nullptr) {
        if (L'T' == buffer_) {
          GetChar();
          result->name = TRUE;
          result->value = L"";
          return true;
        }
        
        return false;
      }

      template <TokenName Name>
      bool Read(Token *result,
                Enabler<Name, NUMBER> *unused = nullptr) {
        if (std::isdigit(buffer_) || L'-' == buffer_) {
          result->name = NUMBER;
          result->value = L"";
          do {
            result->value += buffer_;
            GetChar();
          } while (std::isdigit(buffer_));
          return true;
        }
        
        return false;
      }

      template <TokenName Name>
      bool Read(Token *result,
                Enabler<Name, NIL> *unused = nullptr) {
        if (L'N' == buffer_) {
          result->name = NIL;
          result->value = L"";
          CHECK(L'I' == GetChar());
          CHECK(L'L' == GetChar());
          GetChar();
          return true;
        }
        
        return false;
      }

      // Rely on wifstream to close itself automatically on destruction.
      std::unique_ptr<std::wistream> input_stream_;
      wchar_t buffer_;
      bool end_of_file_;
    };

    class LispObjectReader {
    public:
      LispObjectReader(Tokenizer * tokenizer)
    	: tokenizer_(tokenizer) {}
      
      LispObject Read() {
	// Feeding ReadObject with INVALID_TOKEN indicates ReadObject
	// to not epxect a cached token.
	return ReadObject(Token {INVALID_TOKEN, L""});
      }

    private:
      LispObject ReadObject(Token cache) {

    	Token token;

    	// Get the first token, so that we can decide the type of the
    	// resulting LispObject.
    	if (INVALID_TOKEN == token.name) {
    	  token.Acquire(std::move(cache));
    	} else {
    	  tokenizer_->Next(&token);
    	}

    	switch (token.name) {
    	case NUMBER:
    	  return LispObject::Number(std::stoi(token.value));
    	case STRING:
    	  return LispObject::String(token.value);
    	default:
    	  CHECK(OPEN_PARENTHESIS == token.name);
    	  // Based on the second token, we will decide whether it is
    	  // an object or a list.
    	  CHECK(tokenizer_->Next(&token));
    	  if (KEYWORD == token.name) {
    	    // If we are getting a keyword, it must be an object. We
    	    // do not epxect list with a keyword as the first element.

	    // The LispObject that will be returned as result.
    	    LispObject result = LispObject::Object();

	    // The characters we are reading into token are utf-8
	    // wchars. We need a converter to convert the wchar
	    // strings to normal strings for keys.
	    std::wstring_convert<std::codecvt_utf8<wchar_T>> converter;
	    
    	    std::string key = converter.to_bytes(token.value);

	    bool complete = false;
    	    while (!complete) {
	      result[key] = ReadObject(Token {INVALID_TOKEN, L""});
	      
	      CHECK(tokenizer_->Next(&token));
	      if (CLOSE_PARENTHESIS == token.name) {
		complete = true;
	      } else {
		CHECK(KEYWORD == token.name);
		key = converter.to_bytes(token.value);
	      }
    	    }
	    return result;
    	  } else {  // if (KEYWORD == token.name)
	    // Now we are pretty sure it is a list.
	    LispObject result = LispObject::List();
	    result.Push(ReadObject(token));
	    while (true) {
	      CHECK(tokenizer_->Next(&token));
	      if (CLOSE_PARENTHESIS == token.name) {
		break;
	      } 
	      
	      result.Push(ReadObject(token));
	    }
	    
	    return result;
	  }  // if (KEYWORD == token.name)
    	}
      }
      
      Tokenizer *tokenizer_;
    };

    template <typename Parsable>
    struct ParseList {
      static std::vector<Parsable> Do(parser::Tokenizer *tokenizer,
                                      bool expect_open_paren = true) {
        if (expect_open_paren) {
          parser::TokenName token_name = tokenizer->ExpectOpenParenOrNil();
          if (NIL == token_name) return std::vector<Parsable>();
        }
        parser::Token token;
        bool complete = false;
        std::vector<Parsable> result;
      
        while (!complete) {
          CHECK(tokenizer->Next(&token));
          switch (token.name) {
          case CLOSE_PARENTHESIS:
            complete = true;
            break;
          case OPEN_PARENTHESIS:
            result.emplace_back(Parsable(tokenizer, false));
            break;
          default:
            // Shouldn't have entered here.
            CHECK(false);
          }
        }
        return result;
      }
    };

    template <>
    struct ParseList<int> {
      static std::vector<int> Do(parser::Tokenizer *tokenizer,
                                 bool expect_open_paren = true) {
        if (expect_open_paren) {
          parser::TokenName token_name = tokenizer->ExpectOpenParenOrNil();
          if (NIL == token_name) return std::vector<int>();
        }
        parser::Token token;
        bool complete = false;
        std::vector<int> result;

        while (!complete) {
          CHECK(tokenizer->Next(&token));
          switch (token.name) {
          case CLOSE_PARENTHESIS:
            complete = true;
            break;
          case NUMBER:
            result.push_back(std::stoi(token.value));
            break;
          default:
            // Shouldn't have entered here.
            CHECK(false);
          }
        }
        return result;
      }
    };

    template <>
    struct ParseList<std::wstring> {
      static std::vector<std::wstring> Do(parser::Tokenizer *tokenizer,
                                          bool expect_open_paren = true) {
        if (expect_open_paren) {
          parser::TokenName token_name = tokenizer->ExpectOpenParenOrNil();
          if (NIL == token_name) return std::vector<std::wstring>();
        }
        parser::Token token;
        bool complete = false;
        std::vector<std::wstring> result;

        while (!complete) {
          CHECK(tokenizer->Next(&token));
          switch (token.name) {
          case CLOSE_PARENTHESIS:
            complete = true;
            break;
          case STRING:
            result.push_back(std::move(token.value));
            break;
          default:
            // Shouldn't have entered here.
            CHECK(false);
          }
        }
        return result;
      }
    };
    
  }  // namespace parser
}  // namespace monster_avengers

#endif  // _MONSTER_AVENGERS_MONSTER_DATA_PARSER_