#include "MathParserTestCase.h"

#include "common/utils.h" // IMPLEMENT_STD_HASH_FOR_ENUM_CLASS

#include <unordered_map>

static MathParser::Config config_degrees = { true };
static MathParser::Config config_radians = { false };

MathParserTestCase::MathParserTestCase(const std::string &expression_, double result_, MathParser::Config config_)
: expression(expression_)
, status(MathParser::Status::SUCCESS)
, result(result_)
, config(config_)
{ }

MathParserTestCase::MathParserTestCase(const std::string &expression_, double result_, double current_)
: expression(expression_)
, status(MathParser::Status::SUCCESS)
, result(result_)
, current(current_)
{ }

MathParserTestCase::MathParserTestCase(const std::string &expression_, MathParser::ParsingErrorType parsing_error_)
: expression(expression_)
, status(MathParser::Status::PARSING_ERROR)
, parsing_error(parsing_error_)
{ }

MathParserTestCase::MathParserTestCase(const std::string &expression_, MathParser::EvaluationErrorType evaluation_error_, double current_)
: expression(expression_)
, status(MathParser::Status::EVALUATION_ERROR)
, evaluation_error(evaluation_error_)
, current(current_)
{ }

IMPLEMENT_STD_HASH_FOR_ENUM_CLASS(TrigFunctionType);

typedef double (*unary_function_pointer)(double);

static unary_function_pointer trig_function(TrigFunctionType type) {
  static std::unordered_map<TrigFunctionType, unary_function_pointer> unary_functions = {
    { TrigFunctionType::SIN, &sin },
    { TrigFunctionType::COS, &cos },
    { TrigFunctionType::TAN, &tan },
  };
  return unary_functions[type];
}

MathParserTestCase trig_test_case(const std::string &expression, TrigFunctionType function_type, double angle, TrigAngleUnits units) {
  unary_function_pointer trig = trig_function(function_type);

  switch(units) {
    case TrigAngleUnits::DEGREES: return { expression, trig(common::math::degrees_to_radians<double>() * angle), config_degrees };
    case TrigAngleUnits::RADIANS: return { expression, trig(angle), config_radians };
  }
}
