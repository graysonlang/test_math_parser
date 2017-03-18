#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "MathParser.h"
#include "MathParserTestCase.h"

#include "common/math.h" // common::math::e/pi/tau

#include <cassert> // std::assert
#include <climits> // std::numerical_limis::quiet_NaN()
#include <cmath>   // std::pow
#include <cstdio>  // std::printf
#include <string>
#include <vector>

TEST_CASE("MathParser", "evaluate_expression") {
  static const double E = common::math::e<double>();
  static const double PI = common::math::pi<double>();
  static const double TAU = common::math::tau<double>();

  static std::vector<MathParserTestCase> test_cases = {
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
    { "2x",                          2.0 * 1.0, 1.0 },
    { "3X",                          3.0 * 1.0, 1.0 },
    { "E",                           E },
    { "e",                           E },
    { "pi",                          PI },
    { "Pi",                          PI },
    { "PI",                          PI },
    { "tau",                         TAU },
    { "Tau",                         TAU },
    { "TAU",                         TAU },
    trig_test_case("cos 180",        TrigFunctionType::COS, 180.0),
    trig_test_case("cos(TAU)",       TrigFunctionType::COS, TAU, TrigAngleUnits::RADIANS),
    trig_test_case("sin90.0",        TrigFunctionType::SIN, 90.0),
    trig_test_case("sin(pi / 2)",    TrigFunctionType::SIN, PI / 2.0, TrigAngleUnits::RADIANS),
    trig_test_case("tan45",          TrigFunctionType::TAN, 45.0),
    trig_test_case("tan(e)",         TrigFunctionType::TAN, E, TrigAngleUnits::RADIANS),
  };

  for (const MathParserTestCase &test_case : test_cases) {
    const std::string &expression = test_case.expression;
    std::printf("\"%s\"\n", expression.c_str());

    MathParser::Result result = MathParser::evaluate_expression(expression, test_case.config, test_case.current);

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
            if (result.error_length == 0 || result.filtered_expression.length() == 0) {
              std::printf("<parsing error: syntax error>\n\n");
            } else {
              std::printf("<parsing error: syntax error> at position %zu: \"%s\"\n\n",
                          result.error_position,
                          expression.substr(std::min(expression.length(), result.error_position), result.error_length).c_str());
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

        if (result.error_length == 0 || result.filtered_expression.length() == 0) {
          std::printf("<evaluation error%s>\n\n", error_string.c_str());
        } else {
          std::printf("<evaluation error%s> at position %zu: \"%s\"\n\n",
                      error_string.c_str(),
                      result.error_position,
                      expression.substr(std::min(expression.length(), result.error_position), result.error_length).c_str());
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
