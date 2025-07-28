#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <map>
#include <algorithm>

#include "json.hpp"

#include <boost/multiprecision/cpp_int.hpp>

namespace bm = boost::multiprecision;

struct Point {
    bm::cpp_int x;
    bm::cpp_int y;

    bool operator<(const Point& other) const {
        return x < other.x;
    }
};

nlohmann::json readJsonFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Error: Could not open file: " + filename);
    }
    nlohmann::json data;
    try {
        file >> data;
    } catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error("Error parsing JSON from " + filename + ": " + e.what());
    }
    return data;
}

std::vector<Point> parsePoints(const nlohmann::json& json_data) {
    int k;
    try {
        k = json_data["keys"]["k"].get<int>();
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error("Error: 'k' not found in JSON 'keys' object: " + std::string(e.what()));
    }
    
    std::vector<Point> all_points;

    for (auto it = json_data.begin(); it != json_data.end(); ++it) {
        if (it.key() == "keys") {
            continue;
        }

        bm::cpp_int x_val;
        try {
            x_val = bm::cpp_int(it.key());
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not parse key '" << it.key() << "' as integer. Skipping point. Error: " << e.what() << std::endl;
            continue;
        }

        const auto& point_data = it.value();
        std::string y_value_str;
        int base;

        try {
            y_value_str = point_data["value"].get<std::string>();
            base = point_data["base"].get<int>();
        } catch (const nlohmann::json::exception& e) {
            std::cerr << "Warning: Missing 'value' or 'base' for point x=" << it.key() << ". Skipping point. Error: " << e.what() << std::endl;
            continue;
        }

        bm::cpp_int y_val;
        try {
            y_val = bm::cpp_int(y_value_str, base);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not parse value '" << y_value_str << "' in base " << base << " for x=" << it.key() << ". Skipping point. Error: " << e.what() << std::endl;
            continue;
        }

        all_points.push_back({x_val, y_val});
    }

    std::sort(all_points.begin(), all_points.end());

    if (all_points.size() < k) {
        throw std::runtime_error("Error: Not enough valid points (n=" + std::to_string(all_points.size()) + ") provided in JSON for required k=" + std::to_string(k) + ".");
    }
    
    std::vector<Point> selected_points;
    selected_points.reserve(k);
    for(int i = 0; i < k; ++i) {
        selected_points.push_back(all_points[i]);
    }

    return selected_points;
}

bm::cpp_int calculateSecretC(const std::vector<Point>& points) {
    bm::cpp_int secret_c = 0;
    int k = points.size();
    
    for (int j = 0; j < k; ++j) {
        bm::cpp_int current_x_j = points[j].x;
        bm::cpp_int current_y_j = points[j].y;

        bm::cpp_int numerator_product = current_y_j;
        bm::cpp_int denominator_product = 1;

        for (int i = 0; i < k; ++i) {
            if (i == j) {
                continue;
            }
            bm::cpp_int other_x_i = points[i].x;

            numerator_product *= (-other_x_i); 
            
            bm::cpp_int diff_x = current_x_j - other_x_i;
            if (diff_x == 0) {
                throw std::runtime_error("Error: Duplicate x-values detected. Cannot perform Lagrange interpolation.");
            }
            denominator_product *= diff_x;
        }

        bm::cpp_int term = numerator_product / denominator_product;
        secret_c += term;
    }

    return secret_c;
}

int main() {
    try {
        std::cout << "Processing Test Case 1..." << std::endl;
        nlohmann::json json_data1 = readJsonFile("testcase1.json");
        std::vector<Point> points1 = parsePoints(json_data1);
        bm::cpp_int secret_c1 = calculateSecretC(points1);
        std::cout << "Secret for Test Case 1: " << secret_c1 << std::endl;

        std::cout << "\nProcessing Test Case 2..." << std::endl;
        std::cout << "Note: 'testcase2.json' content was not provided in the prompt. "
                  << "Please ensure the file exists and is correctly formatted if you wish to test it." << std::endl;
        /*
        nlohmann::json json_data2 = readJsonFile("testcase2.json");
        std::vector<Point> points2 = parsePoints(json_data2);
        bm::cpp_int secret_c2 = calculateSecretC(points2);
        std::cout << "Secret for Test Case 2: " << secret_c2 << std::endl;
        */

    } catch (const std::exception& e) {
        std::cerr << "A critical error occurred: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}