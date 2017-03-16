#include "MathParser.h"

#include "common/utils.h" // IMPLEMENT_STD_HASH_FOR_ENUM_CLASS

#include <cassert>
#include <cmath> // std::isnan
#include <cstdio>
#include <locale>
#include <regex>
#include <stack>
#include <unordered_map>

namespace MathParser {

  struct Operator {
    enum class Associativity {
      NONE = 0,
      LEFT,
      RIGHT,
    };

    enum class Type {
      NONE = 0,
      ADD,
      DIVIDE,
      EXPONENT,
      MULTIPLY,
      PAREN_L,
      PAREN_R,
      PERCENTAGE,
      SUBTRACT,
      UNARY_MINUS,
      UNARY_PLUS,
    };

    static const Operator &from_type(Operator::Type type);
    EvaluationErrorType eval(std::stack<double> &values, double current_value = std::numeric_limits<double>::quiet_NaN()) const;
    static const Operator &null_operator();

    Type type;
    Associativity associativity;
    int precedence;
    int degree;
    const char *name;

  private:
    EvaluationErrorType evalBinary(const double &a, const double &b, double &result) const;
  };

  struct Token {
    enum class Type {
      NONE = 0,
      NUMBER,
      OPERATOR,
    };

    enum class Id {
      NONE = 0,
      ASTERISK,
      CARET,
      MINUS,
      PAREN_L,
      PAREN_R,
      PERCENT,
      PLUS,
      SLASH,
    };

    Token(const std::string &string, bool left_is_edge = false);
    static const Token &null_token();

    std::string string;
    size_t position = 0;
    Id id = {};
    Type type = {};
    const Operator &op;
    double value = NAN;

  private:
    static const Id string_to_id(const std::string &string);
    static const Operator &id_to_operator(Id id, bool left_is_edge = false);
    Token();
  };

} // namespace MathParser


IMPLEMENT_STD_HASH_FOR_ENUM_CLASS(MathParser::Operator::Type);
IMPLEMENT_STD_HASH_FOR_ENUM_CLASS(MathParser::Token::Id);


namespace MathParser {

  const Operator &Operator::null_operator() {
    static Operator NULL_OPERATOR = { Operator::Type::NONE, Operator::Associativity::NONE, -1 };
    return NULL_OPERATOR;
  }


  static std::unordered_map<Operator::Type, const Operator> &init_operator_map() {
    static std::unordered_map<Operator::Type, const Operator> map;
    static bool initialized;
    if (!initialized) {
      initialized = true;
      static std::vector<Operator> operators = {
        { Operator::Type::PAREN_L,     Operator::Associativity::NONE,   0, 0, "("   },
        { Operator::Type::PAREN_R,     Operator::Associativity::NONE,   0, 0, ")"   },

        { Operator::Type::ADD,         Operator::Associativity::LEFT,  10, 2, "add" },
        { Operator::Type::SUBTRACT,    Operator::Associativity::LEFT,  10, 2, "sub" },

        { Operator::Type::DIVIDE,      Operator::Associativity::LEFT,  20, 2, "div" },
        { Operator::Type::MULTIPLY,    Operator::Associativity::LEFT,  20, 2, "mul" },

        { Operator::Type::EXPONENT,    Operator::Associativity::RIGHT, 30, 2, "exp" },

        { Operator::Type::PERCENTAGE,  Operator::Associativity::LEFT,  40, 1, "%" },

        { Operator::Type::UNARY_MINUS, Operator::Associativity::RIGHT, 50, 1, "neg" },
        { Operator::Type::UNARY_PLUS,  Operator::Associativity::RIGHT, 50, 1, "pos" },
      };
      for (const Operator &op : operators) {
        map.insert(std::make_pair(op.type, op));
      }
    }
    return map;
  }

  const Operator &Operator::from_type(Operator::Type type) {
    static std::unordered_map<Operator::Type, const Operator> _operator_map = init_operator_map();
    auto it = _operator_map.find(type);
    return it == _operator_map.end() ? null_operator() : it->second;
  }

  EvaluationErrorType Operator::eval(std::stack<double> &values, double current_value) const {
    // Check if we have enough arguments for the operator type.
    if (values.size() < degree) {
      return EvaluationErrorType::EXPECTED_MORE_ARGUMENTS;
    }

    switch (type) {
      case Type::NONE:
      case Type::PAREN_L:
      case Type::PAREN_R:
        return EvaluationErrorType::UNEXPECTED_TOKEN;

      case Type::PERCENTAGE:
      case Type::UNARY_MINUS:
      case Type::UNARY_PLUS: {
        // Handle unary operators.
        double value = values.top();
        values.pop();
        switch(type) {
          default:
            return EvaluationErrorType::UNEXPECTED_TOKEN;

          case Type::PERCENTAGE:
            if (std::isnan(current_value)) {
              return EvaluationErrorType::EXPECTED_CURRENT_VALUE;
            }
            value = value * current_value / 100.0;
            break;

          case Type::UNARY_MINUS: value *= -1.0; break;
          case Type::UNARY_PLUS:  /* no-op */    break;
        }
        values.push(value);
        break;
      }

      case Type::ADD:
      case Type::DIVIDE:
      case Type::EXPONENT:
      case Type::MULTIPLY:       case Type::SUBTRACT: {
        // Handle binary operators.
        double b = std::move(values.top());
        values.pop();
        double a = std::move(values.top());
        values.pop();
        switch(type) {
          default:
            return EvaluationErrorType::UNEXPECTED_TOKEN;
          case Type::ADD:      values.push(a + b); break;
          case Type::SUBTRACT: values.push(a - b); break;
          case Type::DIVIDE:
            if (b == 0.0) {
              return EvaluationErrorType::DIVIDE_BY_ZERO;
            }
            values.push(a / b);
            break;
          case Type::MULTIPLY: values.push(a * b); break;
          case Type::EXPONENT: {
            double d;
            if (a < 0 && std::modf(b, &d) > 0) {
              return EvaluationErrorType::IMAGINARY_NUMBER;
            } else {
              values.push(std::pow(a, b));
            }
            break;
          }
        }
      }
    }
    return EvaluationErrorType::NONE;
  }

  const Token &Token::null_token() {
    static Token NULL_TOKEN = { };
    return NULL_TOKEN;
  }

  Token::Token(const std::string &string_, bool left_is_edge)
  : string(string_)
  , id(string_to_id(string_))
  , type(id == Id::NONE ? Type::NUMBER : Type::OPERATOR)
  , op(id_to_operator(id, left_is_edge))
  {
    if (type == Type::NUMBER) value = atof(string.c_str());
  }

  Token::Token() : string(""), id(Id::NONE), type(Type::NONE), op(Operator::null_operator()) { }

  const Token::Id Token::string_to_id(const std::string &string) {
    static const std::unordered_map<std::string, Token::Id> map = {
      { "*", Token::Id::ASTERISK },
      { "^", Token::Id::CARET },
      { "-", Token::Id::MINUS },
      { "(", Token::Id::PAREN_L },
      { ")", Token::Id::PAREN_R },
      { "%", Token::Id::PERCENT },
      { "+", Token::Id::PLUS },
      { "/", Token::Id::SLASH },
    };
    auto it = map.find(string);
    return (it == map.end()) ? Token::Id::NONE : it->second;
  }

  const Operator &Token::id_to_operator(Id id, bool left_is_edge) {
    Operator::Type type;
    switch(id) {
      case Id::ASTERISK: type = Operator::Type::MULTIPLY;   break;
      case Id::CARET:    type = Operator::Type::EXPONENT;   break;
      case Id::NONE:     type = Operator::Type::NONE;       break;
      case Id::PAREN_L:  type = Operator::Type::PAREN_L;    break;
      case Id::PAREN_R:  type = Operator::Type::PAREN_R;    break;
      case Id::PERCENT:  type = Operator::Type::PERCENTAGE; break;
      case Id::SLASH:    type = Operator::Type::DIVIDE;     break;

        // Differentiate between unary and binary operators.
      case Id::MINUS:    type = left_is_edge ? Operator::Type::UNARY_MINUS : Operator::Type::SUBTRACT; break;
      case Id::PLUS:     type = left_is_edge ? Operator::Type::UNARY_PLUS  : Operator::Type::ADD;      break;
    }
    return Operator::from_type(type);
  }

  Result::Result(ParsingErrorType parsing_error_, std::string &&filtered_expression_, int error_position_, int error_length_)
  : status(Status::PARSING_ERROR)
  , parsing_error(parsing_error_)
  , evaluation_error(EvaluationErrorType::NONE)
  , filtered_expression(filtered_expression_)
  , error_position(error_position_)
  , error_length(error_length_)
  {
  }

  Result::Result(EvaluationErrorType evaluation_error_, std::string &&filtered_expression_, int error_position_, int error_length_)
  : status(Status::EVALUATION_ERROR)
  , parsing_error(ParsingErrorType::NONE)
  , evaluation_error(evaluation_error_)
  , filtered_expression(filtered_expression_)
  , error_position(error_position_)
  , error_length(error_length_)
  {
  }

  Result::Result(double result_)
  : status(Status::SUCCESS)
  , result(result_)
  , parsing_error(ParsingErrorType::NONE)
  , evaluation_error(EvaluationErrorType::NONE)
  , filtered_expression("")
  , error_position(-1)
  , error_length(0)
  {
  }

  // Uses Edsger Dijkstra's "shunting-yard" algorithm to parse math expression.
  // https://en.wikipedia.org/wiki/Shunting-yard_algorithm
  // Expects expression to be formatted with infix notation.
  // Converts into postfix notation and evaluates in place.
  Result evaluate_expression(const std::string &expression, double current_value) {
    static const std::regex pattern_number_or_operator_or_spaces(R"((?:\d*[.]?\d+)(?:e[+\-]?\d+)?|[()+\-*\/^%]|(?:\s+))");
    static const std::regex pattern_number_or_operator          (R"((?:\d*[.]?\d+)(?:e[+\-]?\d+)?|[()+\-*\/^%])");
    static const std::regex spaces(R"(\s+)");

    // Compress whitespace.
    std::string input = std::regex_replace(expression, spaces, " ");

    // Verify input by matching against valid operators, numbers, or spaces.
    std::sregex_token_iterator it(input.begin(), input.end(), pattern_number_or_operator_or_spaces, -1), end;
    while (it != end) {
      unsigned long length = it->length();
      if (length > 0) {
        size_t position = &(*it->first) - input.c_str();
        return { ParsingErrorType::SYNTAX_ERROR, std::move(input), (int)position, (int)length };
      }
      ++it;
    }

    // Info about prior token to disambiguate unary vs binary operators.
    // If the token to the left is the edge of a statement (i.e. left paren, operator, or no token).
    bool left_is_edge = true;

    // Output value stack.
    std::stack<double> output;

    // Operator stack.
    std::stack<Token> stack;

    // Tokenize into operators and numbers.
    it = std::sregex_token_iterator(input.begin(), input.end(), pattern_number_or_operator, 0);

    // Algorithm starts.
    // https://en.wikipedia.org/wiki/Shunting-yard_algorithm#The_algorithm_in_detail
    while(it != end) {
      Token token(it->str(), left_is_edge);
      size_t position = &(*it->first) - input.c_str();
      token.position = position;
      ++it;

      left_is_edge = token.type == Token::Type::NONE || (token.type == Token::Type::OPERATOR && token.op.type != Operator::Type::PAREN_R);

      switch(token.type) {
        case Token::Type::NUMBER: {
          output.push(token.value);
          break;
        }

        case Token::Type::OPERATOR: {
          switch(token.op.type) {
            default:
              while(!stack.empty()) {
                Token &t = stack.top();
                assert(token.op.associativity != Operator::Associativity::NONE);
                if ((token.op.associativity == Operator::Associativity::LEFT && token.op.precedence <= t.op.precedence) ||
                    (token.op.associativity == Operator::Associativity::RIGHT && token.op.precedence < t.op.precedence)) {
                  EvaluationErrorType eval_error = stack.top().op.eval(output, current_value);
                  if (eval_error != EvaluationErrorType::NONE) {
                    return { eval_error, std::move(input), (int)token.position, (int) token.string.length() };
                  }
                  stack.pop();
                }
                else {
                  break;
                }
              }

              stack.push(std::move(token));
              break;

            case Operator::Type::PAREN_L: stack.push(std::move(token)); break;

            case Operator::Type::PAREN_R:
              if (stack.empty()) {
                return { ParsingErrorType::MISMATCHED_PARENS, std::move(input), (int)position };
              }
              while(!stack.empty()) {
                if (stack.top().op.type == Operator::Type::PAREN_L) {
                  stack.pop();
                  break;
                } else {
                  EvaluationErrorType eval_error = stack.top().op.eval(output, current_value);
                  if (eval_error != EvaluationErrorType::NONE) {
                    return { eval_error, std::move(input), (int)token.position, (int) token.string.length() };
                  }
                  stack.pop();
                }

                if (stack.empty()) {
                  return { ParsingErrorType::MISMATCHED_PARENS, std::move(input), (int)position };
                }
              }
              break;
          }
          break;
        }

        case Token::Type::NONE:
          // Should not get here since tokens have already been verified via regular expression.
          return { EvaluationErrorType::UNEXPECTED_TOKEN, std::move(input), (int)position };
      }
    }

    while (!stack.empty()) {
      Token &token = stack.top();
      if (token.op.type == Operator::Type::PAREN_L || token.op.type == Operator::Type::PAREN_L) {
        return { ParsingErrorType::MISMATCHED_PARENS, std::move(input), 0 };
      }
      EvaluationErrorType eval_error = token.op.eval(output, current_value);
      if (eval_error != EvaluationErrorType::NONE) {
        return { eval_error, std::move(input), (int)token.position, (int) token.string.length() };
      }
      stack.pop();
    }
    
    if (output.size() == 0) {
      return { ParsingErrorType::EMPTY };
    } else if (output.size() == 1) {
      return { output.top() };
    } else {
      return { ParsingErrorType::SYNTAX_ERROR };
    }
  }
  
} // namespace MathParser
