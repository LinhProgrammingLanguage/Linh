#pragma once
#include <string>
#include <array>

/* config.hpp
 * This file contains configuration settings for the Linh programming language.
 * It includes version information, author details, default settings, and supported operating systems.
 */

/* Project config */
std::string name = "Linh";
std::string engine = "Linh.cpp";
std::string version = "Alpha 0.1";
std::string version_number = "alpha-0.1";
std::string language = "C++";

/* author */
std::string author = "Jkar / Hoa Tin Develop Team";
std::string copyright = "Copyright (c) 2025 Hoa Tin Develop Team";
std::string web = "https://linh.kesug.com";
std::string email = "linhprogramminglanguage@gmail.com";
std::string github = "https://github.com/LinhProgrammingLanguage/Linh.git";

/* Hoa Tin Develop Team */
std::string hoa_tin_develop_team = "Hoa Tin Develop Team";
std::string hoa_tin_develop_team_web = "https://hoatin.kesug.com";
std::string hoa_tin_develop_team_email = "hoatindev@gmail.com";

/* default config */
/* default number bit */
int number_bit_min = 8;
int number_bit_default = 64;
int number_bit_max = 128;
std::array<int, 5> number_bit_options = {8, 16, 32, 64, 128};
/* default utf*/
std::string default_utf = "utf-8";
/* sp os */
std::array<std::string, 5> sp_os =
    {
        "windows",
        "linux",
        "macos",
        "android",
        "ios"};
/* type of linh */
std::array<std::string, 10> linh_types =
    {
        // basic types
        "int",
        "uint",
        "float",
        "str",
        "bool",
        // objects
        "array",
        "map",
        // ?
        "any",
        "uninit",
        "void"};