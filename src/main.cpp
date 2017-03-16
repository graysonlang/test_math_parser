#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "MathParser.h"

#include <cassert> // std::assert
#include <climits> // std::numerical_limis::quiet_NaN()
#include <cmath>   // std::pow
#include <cstdio>  // std::printf
#include <string>
#include <vector>

using MathParser::EvaluationErrorType;
using MathParser::ParsingErrorType;

TEST_CASE("MathParser", "evaluate_expression") {
  struct MathTestCase {
    std::string expression;
    MathParser::Status status = { };
    double current = std::numeric_limits<double>::quiet_NaN();;
    double result = std::numeric_limits<double>::quiet_NaN();
    MathParser::ParsingErrorType parsing_error = { };
    MathParser::EvaluationErrorType evaluation_error = { };


    MathTestCase(const std::string &expression_, double result_, double current_ = std::numeric_limits<double>::quiet_NaN())
    : expression(expression_)
    , status(MathParser::Status::SUCCESS)
    , result(result_)
    , current(current_)
    { }

    MathTestCase(const std::string &expression_, MathParser::ParsingErrorType parsing_error_)
    : expression(expression_)
    , status(MathParser::Status::PARSING_ERROR)
    , parsing_error(parsing_error_)
    { }

    MathTestCase(const std::string &expression_, MathParser::EvaluationErrorType evaluation_error_, double current_ = std::numeric_limits<double>::quiet_NaN())
    : expression(expression_)
    , status(MathParser::Status::EVALUATION_ERROR)
    , evaluation_error(evaluation_error_)
    , current(current_)
    { }
  };

  static std::vector<MathTestCase> test_cases = {
    { "",                            ParsingErrorType::EMPTY },
    { " \f\n\r\t\v",                 ParsingErrorType::EMPTY },
    { "()",                          ParsingErrorType::EMPTY },
    { "1(1+",                        ParsingErrorType::MISMATCHED_PARENS },
    { "((1)",                        ParsingErrorType::MISMATCHED_PARENS },
    { "(1))",                        ParsingErrorType::MISMATCHED_PARENS },
    { "1 + (2 - (3 * (4 / (5)))))",  ParsingErrorType::MISMATCHED_PARENS },
    { "(1)1",                        ParsingErrorType::SYNTAX_ERROR },
    { "1a",                          ParsingErrorType::SYNTAX_ERROR },
    { "abc",                         ParsingErrorType::SYNTAX_ERROR },
    { "a + b * c",                   ParsingErrorType::SYNTAX_ERROR },
    { "1 2 3",                       ParsingErrorType::SYNTAX_ERROR },
    { "12.",                         ParsingErrorType::SYNTAX_ERROR },
    { "1 + 2 # 3",                   ParsingErrorType::SYNTAX_ERROR },

    { "1 / (1 - 1)",                 EvaluationErrorType::DIVIDE_BY_ZERO },
    { "50%",                         EvaluationErrorType::EXPECTED_CURRENT_VALUE },
    { "+",                           EvaluationErrorType::EXPECTED_MORE_ARGUMENTS },
    { "1 *",                         EvaluationErrorType::EXPECTED_MORE_ARGUMENTS },
    { "(1 + ) + 1",                  EvaluationErrorType::EXPECTED_MORE_ARGUMENTS },
    { "-",                           EvaluationErrorType::EXPECTED_MORE_ARGUMENTS },
    { "--",                          EvaluationErrorType::EXPECTED_MORE_ARGUMENTS },
    { "-1 ^ 2 ^ 3.4",                EvaluationErrorType::IMAGINARY_NUMBER },

    { "1",                           1 },
    { "123",                         123 },
    { "1.23",                        1.23 },
    { ".12",                         .12 },
    { "1e2",                         1e2 },
    { "1e+2 + 3",                    1e+2 + 3 },
    { "1e-2 - 3",                    1e-2 - 3 },
    { "+1",                          1 },
    { "++1",                         1 },
    { "+++1",                        1 },
    { "-1",                          -1 },
    { "--1",                         1 },
    { "---1",                        -1 },
    { "((1))",                       1 },
    { "1 + 2",                       1 + 2 },
    { "1 + (2)",                     1 + (2) },
    { "(1) + 2",                     (1) + 2 },
    { "+(1 + 2)",                    +(1 + 2)},
    { "-(1 - 2)",                    -(1 - 2)},
    { "1 + 2 * 3",                   1 + 2 * 3 },
    { "1 + (2 * 3)",                 1 + (2 * 3) },
    { "(1 + 2) * 3",                 (1 + 2) * 3 },
    { "-1 ^ 2",                      std::pow(-1, 2) },
    { "(-1) ^ 2",                    std::pow(-1, 2) },
    { "-(1 ^ 2)",                    -std::pow(1, 2) },
    { "4 ^ -2",                      std::pow(4, -2) },
    { "(-4 ^ 2)",                    std::pow(-4, 2) },
    { "2 * 2 ^ 3",                   2 * std::pow(2, 3) },
    { "2 * (2 ^ 3)",                 2 * std::pow(2, 3) },
    { "(2 * 2) ^ 3",                 std::pow(2 * 2, 3) },
    { "2 ^ 2 ^ 3",                   std::pow(2, std::pow(2, 3)) },
    { "2 ^ (2 ^ 3)",                 std::pow(2, std::pow(2, 3)) },
    { "(2 ^ 2) ^ 3",                 std::pow(std::pow(2, 2), 3) },
    { "1 + .2 * -3 / +4 ^ 5",        1 + .2 * -3 / std::pow(+4, 5) },
    { "+-+-1++--++--++--+2-3+4",     1+2-3+4 },
    { "50%",                         0.5 * 1.0, 1.0 },
  };

  for (const MathTestCase &test_case : test_cases) {
    const std::string &expression = test_case.expression;
    std::printf("\"%s\"\n", expression.c_str());

    MathParser::Result result = MathParser::evaluate_expression(expression, test_case.current);

    switch(result.status) {
      case MathParser::Status::SUCCESS: {
        std::printf("= %-10.10g\n\n", result.result);
        break;
      }

      case MathParser::Status::PARSING_ERROR: {
        switch(result.parsing_error) {
          case MathParser::ParsingErrorType::NONE:
            assert(false);
            break;

          case MathParser::ParsingErrorType::EMPTY:
            std::printf("<parsing error: empty>\n\n");
            break;

          case MathParser::ParsingErrorType::MISMATCHED_PARENS:
            std::printf("<parsing error: mismatched parens>\n\n");
            break;

          case MathParser::ParsingErrorType::SYNTAX_ERROR: {
            if (result.error_position < 0) {
              std::printf("<parsing error: syntax error>\n\n");
            } else {
              std::printf("<parsing error: syntax error> at position %i: \"%s\"\n\n",
                     result.error_position,
                     expression.substr(std::min((int)expression.length(), result.error_position), result.error_length).c_str());
            }
            break;
          }
        }
        break;
      }

      case MathParser::Status::EVALUATION_ERROR: {
        std::string error_string = "";

        switch(result.evaluation_error) {
          case MathParser::EvaluationErrorType::NONE: {
            assert(false);
            break;
          }

          case MathParser::EvaluationErrorType::DIVIDE_BY_ZERO:          error_string = ": divide by zero";          break;
          case MathParser::EvaluationErrorType::EXPECTED_CURRENT_VALUE:  error_string = ": expected current value";  break;
          case MathParser::EvaluationErrorType::EXPECTED_MORE_ARGUMENTS: error_string = ": expected more arguments"; break;
          case MathParser::EvaluationErrorType::IMAGINARY_NUMBER:        error_string = ": imaginary number";        break;
          case MathParser::EvaluationErrorType::UNEXPECTED_TOKEN:        error_string = ": unexpected token";        break;
        }

        if (result.error_position < 0) {
          std::printf("<evaluation error%s>\n\n", error_string.c_str());
        } else {
          std::printf("<evaluation error%s> at position %i: \"%s\"\n\n",
                 error_string.c_str(),
                 result.error_position,
                 expression.substr(std::min((int)expression.length(), result.error_position), result.error_length).c_str());
        }
        break;
      }
    }

    REQUIRE(result.status == test_case.status);
    switch(result.status) {
      case MathParser::Status::SUCCESS:
        REQUIRE(result.result == test_case.result);
        break;

      case MathParser::Status::PARSING_ERROR:
        REQUIRE( result.parsing_error == test_case.parsing_error);
        break;

      case MathParser::Status::EVALUATION_ERROR:
        REQUIRE( result.evaluation_error == test_case.evaluation_error);
        break;
    }
  }
}
