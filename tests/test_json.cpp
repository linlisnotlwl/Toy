#include <iostream>
#include <cstdio>
#include <cstring>
#include <chrono>
#include "../lib/Json.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "ERROR:@ %s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")
#define EXPECT_EQ_STRING(expect, actual, alength) \
    EXPECT_EQ_BASE(sizeof(expect) - 1 == alength && memcmp(expect, actual, alength) == 0, expect, actual, "%s")
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")

#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%zu")


static void test_parse_null() {
    Toy::JsonVar v;
    v.setType(Toy::JsonVar::FALSE);
    EXPECT_EQ_INT(Toy::Json::ParseStatus::OK, Toy::Json::parse("null", &v));
    EXPECT_EQ_INT(Toy::JsonVar::NULL_TYPE, v.getType());
}

static void test_parse_true() {
    Toy::JsonVar v;
    v.setType(Toy::JsonVar::FALSE);
    EXPECT_EQ_INT(Toy::Json::ParseStatus::OK, Toy::Json::parse("true", &v));
    EXPECT_EQ_INT(Toy::JsonVar::TRUE, v.getType());
}

static void test_parse_false() {
    Toy::JsonVar v;
    v.setType(Toy::JsonVar::TRUE);
    EXPECT_EQ_INT(Toy::Json::ParseStatus::OK, Toy::Json::parse("false", &v));
    EXPECT_EQ_INT(Toy::JsonVar::FALSE, v.getType());
}



#define TEST_NUMBER(expect, json)\
    do {\
        Toy::JsonVar v;\
        EXPECT_EQ_INT(Toy::Json::ParseStatus::OK, Toy::Json::parse(json, &v));\
        EXPECT_EQ_INT(Toy::JsonVar::NUMBER, v.getType());\
        EXPECT_EQ_DOUBLE(expect, v.getNumberVal());\
    } while(0)

static void test_parse_number() {
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

#define TEST_STRING(expect, json)\
    do {\
        Toy::JsonVar v;\
        v.setType(Toy::JsonVar::STRING);\
        EXPECT_EQ_INT(Toy::Json::ParseStatus::OK, Toy::Json::parse(json, &v));\
        EXPECT_EQ_INT(Toy::JsonVar::STRING, v.getType());\
        EXPECT_EQ_STRING(expect, v.getCStr(), v.getCStrLength());\
    } while(0)

static void test_parse_string() {
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
    TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
    TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
    TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
    TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
}

static void test_parse_array() {
    Toy::JsonVar v;

    v.setType(Toy::JsonVar::ARRAY);
    EXPECT_EQ_INT(Toy::Json::ParseStatus::OK, Toy::Json::parse("[ ]", &v));
    EXPECT_EQ_INT(Toy::JsonVar::ARRAY, v.getType());
    EXPECT_EQ_SIZE_T(0, v.getArraySize());

    EXPECT_EQ_INT(Toy::Json::ParseStatus::OK, Toy::Json::parse("[[1, false, null], 1 ]", &v));
    EXPECT_EQ_SIZE_T(2, v.getArraySize());

    EXPECT_EQ_INT(Toy::Json::ParseStatus::OK, Toy::Json::parse("[ null , false , true , 123 , \"abc\" ]", &v));
    EXPECT_EQ_SIZE_T(5, v.getArraySize());

    EXPECT_EQ_INT(Toy::JsonVar::NULL_TYPE,  v.getArrayElememt(0)->getType());
    EXPECT_EQ_INT(Toy::JsonVar::FALSE, v.getArrayElememt(1)->getType());
    EXPECT_EQ_INT(Toy::JsonVar::TRUE, v.getArrayElememt(2)->getType());
    EXPECT_EQ_INT(Toy::JsonVar::NUMBER, v.getArrayElememt(3)->getType());
    EXPECT_EQ_INT(Toy::JsonVar::STRING, v.getArrayElememt(4)->getType());
    EXPECT_EQ_DOUBLE(123.0, v.getArrayElememt(3)->getNumberVal());
    EXPECT_EQ_STRING("abc", v.getArrayElememt(4)->getCStr(), v.getArrayElememt(4)->getCStrLength());
    
    EXPECT_EQ_INT(Toy::Json::ParseStatus::OK, Toy::Json::parse("[ [  ], [ 1 ], [ 2 ] ]", &v));
    EXPECT_EQ_INT(Toy::Json::ParseStatus::OK, Toy::Json::parse("[ [ 0 , false ] , [ 0 , null , \"hello\" ] ]", &v));
    EXPECT_EQ_INT(Toy::Json::ParseStatus::OK, Toy::Json::parse("[ [ 0 , [] ] , [ 0 , null , \"hello\" ] ]", &v));

    Toy::JsonVar v1;
    v1.setType(Toy::JsonVar::ARRAY);
    EXPECT_EQ_INT(Toy::Json::ParseStatus::OK, Toy::Json::parse("[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]", &v1));
    EXPECT_EQ_SIZE_T(4, v1.getArraySize());
    size_t i, j;
    for (i = 0; i < 4; i++) {
        Toy::JsonVar* a = v1.getArrayElememt(i);
        EXPECT_EQ_INT(Toy::JsonVar::ARRAY, a->getType());
        EXPECT_EQ_SIZE_T(i, a->getArraySize());
        for (j = 0; j < i; j++) {
            Toy::JsonVar* e = a->getArrayElememt(j);
            EXPECT_EQ_INT(Toy::JsonVar::NUMBER, e->getType());
            EXPECT_EQ_DOUBLE((double)j, e->getNumberVal());
        }
    }
}

static void test_parse_object() {
    Toy::JsonVar v;
    size_t i;

    EXPECT_EQ_INT(Toy::Json::ParseStatus::OK, Toy::Json::parse(" { } ", &v));
    EXPECT_EQ_INT(Toy::JsonVar::OBJECT, v.getType());
    EXPECT_EQ_SIZE_T(0, v.getObjectSize());

    EXPECT_EQ_INT(Toy::Json::ParseStatus::OK, Toy::Json::parse(
        " { \"n\" : null , \"f\" : false , \"t\" : true , \"i\" : 123 , \"s\" : \"abc\", \"a\" : [ 1, 2, 3 ],\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 } } ",&v));
    EXPECT_EQ_INT(Toy::JsonVar::OBJECT, v.getType());
    EXPECT_EQ_SIZE_T(7, v.getObjectSize());
    // EXPECT_EQ_STRING("n", lept_get_object_key(&v, 0), lept_get_object_key_length(&v, 0));
    // EXPECT_EQ_INT(Toy::JsonVar::NULL,   lept_get_type(lept_get_object_value(&v, 0)));
    // EXPECT_EQ_STRING("f", lept_get_object_key(&v, 1), lept_get_object_key_length(&v, 1));
    // EXPECT_EQ_INT(Toy::JsonVar::FALSE,  lept_get_type(lept_get_object_value(&v, 1)));
    // EXPECT_EQ_STRING("t", lept_get_object_key(&v, 2), lept_get_object_key_length(&v, 2));
    // EXPECT_EQ_INT(Toy::JsonVar::TRUE,   lept_get_type(lept_get_object_value(&v, 2)));
    // EXPECT_EQ_STRING("i", lept_get_object_key(&v, 3), lept_get_object_key_length(&v, 3));
    // EXPECT_EQ_INT(Toy::JsonVar::NUMBER, lept_get_type(lept_get_object_value(&v, 3)));
    // EXPECT_EQ_DOUBLE(123.0, lept_get_number(lept_get_object_value(&v, 3)));
    // EXPECT_EQ_STRING("s", lept_get_object_key(&v, 4), lept_get_object_key_length(&v, 4));
    // EXPECT_EQ_INT(Toy::JsonVar::STRING, lept_get_type(lept_get_object_value(&v, 4)));
    // EXPECT_EQ_STRING("abc", lept_get_string(lept_get_object_value(&v, 4)), lept_get_string_length(lept_get_object_value(&v, 4)));
    // EXPECT_EQ_STRING("a", lept_get_object_key(&v, 5), lept_get_object_key_length(&v, 5));
    // EXPECT_EQ_INT(Toy::JsonVar::ARRAY, lept_get_type(lept_get_object_value(&v, 5)));
    // EXPECT_EQ_SIZE_T(3, lept_get_array_size(lept_get_object_value(&v, 5)));
    for (i = 0; i < 3; i++) {
        Toy::JsonVar* e = v.getObjectValue("a")->getArrayElememt(i);
        EXPECT_EQ_INT(Toy::JsonVar::NUMBER, e->getType());
        EXPECT_EQ_DOUBLE(i + 1.0, e->getNumberVal());
    }
    //EXPECT_EQ_STRING("o", lept_get_object_key(&v, 6), lept_get_object_key_length(&v, 6));
    {
        Toy::JsonVar* o = v.getObjectValue("o");
        EXPECT_EQ_INT(Toy::JsonVar::OBJECT, o->getType());
        
        Toy::JsonVar* ov1 = o->getObjectValue("1");
        EXPECT_EQ_SIZE_T(1, ov1->getNumberVal());
        EXPECT_EQ_INT(Toy::JsonVar::NUMBER, ov1->getType());
        EXPECT_EQ_DOUBLE((double)1, ov1->getNumberVal());
        
    }
}

#define TEST_ERROR(error, json)\
    do {\
        Toy::JsonVar v;\
        v.setType(Toy::JsonVar::FALSE);\
        EXPECT_EQ_INT(error, Toy::Json::parse(json, &v));\
        EXPECT_EQ_INT(Toy::JsonVar::NULL_TYPE, v.getType());\
    } while(0)

static void test_parse_expect_value() {
    TEST_ERROR(Toy::Json::EXPECT_VALUE, "");
    TEST_ERROR(Toy::Json::EXPECT_VALUE, " ");
}

static void test_parse_root_not_singular() {
    TEST_ERROR(Toy::Json::ParseStatus::ROOT_NOT_SINGULAR, "null x");
    /* invalid number */
    /* after zero should be '.' or nothing */
    TEST_ERROR(Toy::Json::ParseStatus::ROOT_NOT_SINGULAR, "0123"); 
    TEST_ERROR(Toy::Json::ParseStatus::ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(Toy::Json::ParseStatus::ROOT_NOT_SINGULAR, "0x123");
    TEST_ERROR(Toy::Json::ParseStatus::ROOT_NOT_SINGULAR, "1.8b");
}

static void test_parse_number_too_big() {
    TEST_ERROR(Toy::Json::ParseStatus::NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(Toy::Json::ParseStatus::NUMBER_TOO_BIG, "-1e309");
}

static void test_parse_invalid_value() {
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_VALUE, "nul");
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_VALUE, "?");
    /* ... */
    /* invalid number */
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_VALUE, "+0");
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_VALUE, "+1");
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_VALUE, ".123"); /* at least one digit before '.' */
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_VALUE, "1.");   /* at least one digit after '.' */
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_VALUE, "INF");
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_VALUE, "inf");
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_VALUE, "NAN");
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_VALUE, "nan");

    TEST_ERROR(Toy::Json::ParseStatus::INVALID_VALUE, "-.1");
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_VALUE, "e-100");

    TEST_ERROR(Toy::Json::ParseStatus::INVALID_VALUE, "[1,]");
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_VALUE, "[\"a\", nul]");

}
static void test_parse_missing_quotation_mark() {
    TEST_ERROR(Toy::Json::MISS_QUOTATION_MARK, "\"");
    TEST_ERROR(Toy::Json::MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape() {
    TEST_ERROR(Toy::Json::ParseStatus::MISS_QUOTATION_MARK, "\"\\v\"");
    TEST_ERROR(Toy::Json::ParseStatus::MISS_QUOTATION_MARK, "\"\\'\"");
    TEST_ERROR(Toy::Json::ParseStatus::MISS_QUOTATION_MARK, "\"\\0\"");
    TEST_ERROR(Toy::Json::ParseStatus::MISS_QUOTATION_MARK, "\"\\x12\"");
}

static void test_parse_invalid_string_char() {
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_STRING_CHAR, "\"\x1F\"");
}

static void test_parse_invalid_unicode_hex() {
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_UNICODE_HEX, "\"\\u\"");
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_UNICODE_HEX, "\"\\u0\"");
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_UNICODE_HEX, "\"\\u01\"");
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_UNICODE_HEX, "\"\\u012\"");
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_UNICODE_HEX, "\"\\u/000\"");
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_UNICODE_HEX, "\"\\uG000\"");
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_UNICODE_HEX, "\"\\u0G00\"");
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_UNICODE_HEX, "\"\\u00/0\"");
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_UNICODE_HEX, "\"\\u00G0\"");
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_UNICODE_HEX, "\"\\u000/\"");
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_UNICODE_HEX, "\"\\u000G\"");
}

static void test_parse_invalid_unicode_surrogate() {
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
    TEST_ERROR(Toy::Json::ParseStatus::INVALID_UNICODE_SURROGATE, "\"\\uE000\"");
}

static void test_parse_miss_comma_or_square_bracket() {
    TEST_ERROR(Toy::Json::ParseStatus::MISS_COMMA_OR_SQUARE_BRACKET, "[1");
    TEST_ERROR(Toy::Json::ParseStatus::MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
    TEST_ERROR(Toy::Json::ParseStatus::MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
    TEST_ERROR(Toy::Json::ParseStatus::MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
}

static void test_parse_miss_key() {
    TEST_ERROR(Toy::Json::ParseStatus::MISS_KEY, "{:1,");
    TEST_ERROR(Toy::Json::ParseStatus::MISS_KEY, "{1:1,");
    TEST_ERROR(Toy::Json::ParseStatus::MISS_KEY, "{true:1,");
    TEST_ERROR(Toy::Json::ParseStatus::MISS_KEY, "{false:1,");
    TEST_ERROR(Toy::Json::ParseStatus::MISS_KEY, "{null:1,");
    TEST_ERROR(Toy::Json::ParseStatus::MISS_KEY, "{[]:1,");
    TEST_ERROR(Toy::Json::ParseStatus::MISS_KEY, "{{}:1,");
    TEST_ERROR(Toy::Json::ParseStatus::MISS_KEY, "{\"a\":1,");
}

static void test_parse_miss_colon() {
    TEST_ERROR(Toy::Json::ParseStatus::MISS_COLON, "{\"a\"}");
    TEST_ERROR(Toy::Json::ParseStatus::MISS_COLON, "{\"a\",\"b\"}");
}

static void test_parse_miss_comma_or_curly_bracket() {
    TEST_ERROR(Toy::Json::ParseStatus::MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
    TEST_ERROR(Toy::Json::ParseStatus::MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
    TEST_ERROR(Toy::Json::ParseStatus::MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
    TEST_ERROR(Toy::Json::ParseStatus::MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
}

static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_expect_value();
    test_parse_root_not_singular();
    test_parse_number();
    test_parse_invalid_value();
    test_parse_number_too_big();
    test_parse_string();
    test_parse_invalid_string_escape();
    test_parse_missing_quotation_mark();
    test_parse_invalid_string_char();
    test_parse_invalid_unicode_hex();
    test_parse_invalid_unicode_surrogate();
    test_parse_array();
    test_parse_miss_comma_or_square_bracket();
    test_parse_miss_key();
    test_parse_miss_colon();
    test_parse_miss_comma_or_curly_bracket();
    test_parse_object();   
}

#define TEST_EQUAL(json1, json2, equality) \
    do {\
        Toy::JsonVar v1, v2;\
        EXPECT_EQ_INT(Toy::Json::ParseStatus::OK, Toy::Json::parse(json1, &v1));\
        EXPECT_EQ_INT(Toy::Json::ParseStatus::OK, Toy::Json::parse(json2, &v2));\
        EXPECT_EQ_INT(equality, v1 == v2);\
    } while(0)

static void test_equal() {
    TEST_EQUAL("true", "true", true);
    TEST_EQUAL("true", "false", false);
    TEST_EQUAL("false", "false", true);
    TEST_EQUAL("null", "null", true);
    TEST_EQUAL("null", "0", false);
    TEST_EQUAL("123", "123", true);
    TEST_EQUAL("123", "456", false);
    TEST_EQUAL("\"abc\"", "\"abc\"", true);
    TEST_EQUAL("\"abc\"", "\"abcd\"", false);
    TEST_EQUAL("[]", "[]", true);
    TEST_EQUAL("[]", "null", false);
    TEST_EQUAL("[1,2,3]", "[1,2,3]", true);
    TEST_EQUAL("[1,2,3]", "[1,2,3,4]", false);
    TEST_EQUAL("[[]]", "[[]]", true);
    TEST_EQUAL("{}", "{}", true);
    TEST_EQUAL("{}", "null", false);
    TEST_EQUAL("{}", "[]", false);
    TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":2}", true);
    TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"b\":2,\"a\":1}", true);
    TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":3}", false);
    TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":2,\"c\":3}", false);
    TEST_EQUAL("{\"a\":{\"b\":{\"c\":{}}}}", "{\"a\":{\"b\":{\"c\":{}}}}", true);
    TEST_EQUAL("{\"a\":{\"b\":{\"c\":{}}}}", "{\"a\":{\"b\":{\"c\":[]}}}", false);
}

static void test_copy() {
    Toy::JsonVar v1;
    Toy::Json::parse("{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}", &v1);
    //Toy::Json::parse("[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]", &v1);
    Toy::JsonVar v2(v1);
    EXPECT_TRUE(v1 == v2);
    
}

static void test_move() {
    Toy::JsonVar v1;
    Toy::Json::parse("{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}", &v1);
    Toy::JsonVar v2(v1);
    Toy::JsonVar v3(std::move(v2));
    EXPECT_EQ_INT(Toy::JsonVar::NULL_TYPE, v2.getType());
    EXPECT_TRUE(v1 == v3);
}

static void test_swap() {
    Toy::JsonVar v1, v2;
    v1.setType(Toy::JsonVar::STRING);
    v2.setType(Toy::JsonVar::STRING);
    v1.setCStr("Hello", 5);
    v2.setCStr("World!", 6);
    swap(v1, v2);
    EXPECT_EQ_STRING("World!", v1.getCStr(), v1.getCStrLength());
    EXPECT_EQ_STRING("Hello",  v2.getCStr(), v2.getCStrLength());
}

static void test_access_null() {
    Toy::JsonVar v;
    v.setType(Toy::JsonVar::STRING);
    v.setCStr("a", 1);
    v.setType(Toy::JsonVar::NULL_TYPE);
    EXPECT_EQ_INT(Toy::JsonVar::NULL_TYPE, v.getType());
}

static void test_access_boolean() {
    Toy::JsonVar v;
    v.setType(Toy::JsonVar::STRING);
    v.setCStr("a", 1);
    v.setType(Toy::JsonVar::FALSE);
    v.setBoolVal(false);
    EXPECT_FALSE(v.getBoolVal());
    v.setType(Toy::JsonVar::TRUE);
    v.setBoolVal(true);
    EXPECT_TRUE(v.getBoolVal());
}

static void test_access_number() {
    Toy::JsonVar v;
    v.setType(Toy::JsonVar::STRING);
    v.setCStr("a", 1);
    v.setType(Toy::JsonVar::NUMBER);
    v.setNumberVal(-1.0);
    TEST_NUMBER(v.getNumberVal(), "-1.0");
    v.setNumberVal(1.5);
    TEST_NUMBER(v.getNumberVal(), "1.5");
    v.setNumberVal(-1.5);
    TEST_NUMBER(v.getNumberVal(), "-1.5");
    v.setNumberVal(3.1416);
    TEST_NUMBER(v.getNumberVal(), "3.1416");
}

static void test_access_string() {
    Toy::JsonVar v;
    v.setType(Toy::JsonVar::STRING);
    v.setCStr("", 0);
    EXPECT_EQ_STRING("", v.getCStr(), v.getCStrLength());
    v.setCStr("Hello", 5);
    EXPECT_EQ_STRING("Hello", v.getCStr(), v.getCStrLength());
}

static void test_access_array() {
    Toy::JsonVar a(Toy::JsonVar::ARRAY);
    size_t i;
    for (i = 0; i < 3; i++) {
        Toy::JsonVar e(Toy::JsonVar::NUMBER); 
        e.setNumberVal(i);
        a.addArrayElement(std::move(e));
    }
    
    EXPECT_EQ_SIZE_T(3, a.getArraySize());
    for (i = 0; i < 3; i++)
        EXPECT_EQ_DOUBLE((double)i, a[i].getNumberVal());

    Toy::JsonVar e(Toy::JsonVar::STRING); 
    e.setCStr("Hello", 5);
    a.addArrayElement(e);
    EXPECT_EQ_STRING("Hello", e.getCStr(), e.getCStrLength());
    EXPECT_EQ_SIZE_T(4, a.getArraySize());
    EXPECT_EQ_STRING("Hello", a[3].getCStr(), a[3].getCStrLength());

}

static void test_access_object() {
    Toy::JsonVar o(Toy::JsonVar::OBJECT);
    Toy::JsonVar v(Toy::JsonVar::NUMBER);
    size_t i;

    for (i = 0; i < 10; i++)
    {
        v.setType(Toy::JsonVar::NUMBER);
        char key[2] = "a";
        key[0] += i;
        v.setNumberVal(i);
        o.addObjectElement(std::move(std::string(key)), std::move(v));
    }

    EXPECT_EQ_SIZE_T(10, o.getObjectSize());
    for (i = 0; i < 10; i++)
    {
        char key[] = "a";
        key[0] += i;
        Toy::JsonVar * temp1 = o.getObjectValue(std::string(key));
        Toy::JsonVar temp2 = o[std::string(key)];
        EXPECT_EQ_DOUBLE((double)i, temp1->getNumberVal());
        EXPECT_EQ_DOUBLE((double)i, temp2.getNumberVal());
    }

    for (i = 0; i < 10; i++) {
        char key[] = "a";
        key[0] += i;
        o[std::string(key)].setNumberVal(i + 1);
    }

    v.setType(Toy::JsonVar::STRING);
    v.setCStr("Hello", 5);
    o.addObjectElement(std::string("World"), v);

    EXPECT_EQ_STRING("Hello", o["World"].getCStr(), o["World"].getCStrLength());
    EXPECT_EQ_STRING("Hello", v.getCStr(), v.getCStrLength());
}

static void test_access()
{
    test_access_null();
    test_access_boolean();
    test_access_number();
    test_access_string();
    test_access_array();
    test_access_object();
}

#define TEST_ROUNDTRIP(json)\
    do {\
        Toy::JsonVar v;\
        EXPECT_EQ_INT(Toy::Json::ParseStatus::OK, Toy::Json::parse(json, &v));\
        std::string temp = Toy::Json::stringify(&v);\
        size_t length = temp.length();\
        const char * json2 = temp.c_str();\
        EXPECT_EQ_STRING(json, json2, length);\
    } while(0)

static void test_stringify_number() {
    TEST_ROUNDTRIP("0");
    TEST_ROUNDTRIP("-0");
    TEST_ROUNDTRIP("1");
    TEST_ROUNDTRIP("-1");
    TEST_ROUNDTRIP("1.5");
    TEST_ROUNDTRIP("-1.5");
    TEST_ROUNDTRIP("3.25");
    TEST_ROUNDTRIP("1e+20");
    TEST_ROUNDTRIP("1.234e+20");
    TEST_ROUNDTRIP("1.234e-20");

    TEST_ROUNDTRIP("1.0000000000000002"); /* the smallest number > 1 */
    TEST_ROUNDTRIP("4.9406564584124654e-324"); /* minimum denormal */
    TEST_ROUNDTRIP("-4.9406564584124654e-324");
    TEST_ROUNDTRIP("2.2250738585072009e-308");  /* Max subnormal double */
    TEST_ROUNDTRIP("-2.2250738585072009e-308");
    TEST_ROUNDTRIP("2.2250738585072014e-308");  /* Min normal positive double */
    TEST_ROUNDTRIP("-2.2250738585072014e-308");
    TEST_ROUNDTRIP("1.7976931348623157e+308");  /* Max double */
    TEST_ROUNDTRIP("-1.7976931348623157e+308");
}

static void test_stringify_string() {
    TEST_ROUNDTRIP("\"\"");
    TEST_ROUNDTRIP("\"Hello\"");
    TEST_ROUNDTRIP("\"Hello\\nWorld\"");
    TEST_ROUNDTRIP("\"\\\" \\\\ / \\b \\f \\n \\r \\t\""); // \" \\ / \b \f \n \r \t
    TEST_ROUNDTRIP("\"Hello\\u0000World\"");
}

static void test_stringify_array() {
    TEST_ROUNDTRIP("[]");
    TEST_ROUNDTRIP("[null,false,true,123,\"abc\",[1,2,3]]");
}

static void test_stringify_object() {
    TEST_ROUNDTRIP("{}");
    //TEST_ROUNDTRIP("{\"n\":null,\"f\":false,\"t\":true,\"i\":123,\"s\":\"abc\",\"a\":[1,2,3],\"o\":{\"1\":1,\"2\":2,\"3\":3}}");
    // TODO : sequence has not been considered becasue of using multimap
    TEST_ROUNDTRIP("{\"a\":[1,2,3],\"f\":false,\"i\":123,\"n\":null,\"o\":{\"1\":1,\"2\":2,\"3\":3},\"s\":\"abc\",\"t\":true}");
    
}

static void test_stringify() {
    TEST_ROUNDTRIP("null");
    TEST_ROUNDTRIP("false");
    TEST_ROUNDTRIP("true");
    test_stringify_number();
    test_stringify_string();
    test_stringify_array();
    test_stringify_object();
}

int main()
{
    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < 20000; ++i)
    {
        test_parse();
        test_stringify();
        test_equal();
        test_copy();
        test_move();
        test_swap();
        test_access();
    }
    auto end = std::chrono::high_resolution_clock::now();
    printf("TEST RESULT:%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    std::cout << "Time: " 
        << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() 
        << " us" << std::endl;
    return 0;
}