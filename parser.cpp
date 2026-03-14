#include "haversine.hpp"

struct pair {
  f64 x0;
  f64 y0;
  f64 x1;
  f64 y1;
};

bool is_whitespace(char c) {
  const u64 num_whitespace = 4;
  const char whitespace[num_whitespace] = {
    ' ', '\n', '\r', '\t'
  };
  for (u64 i{}; i < num_whitespace; i++) {
    if (c == whitespace[i]) return true;
  }
  return false;
}

void next_non_whitespace(std::ifstream& json, char& c) { // TODO: combine this + panic_if with expected next char
  json.get(c);
  while (is_whitespace(c)) { json.get(c); }
}

void check_next_non_whitespace(std::ifstream& json, char& c, const char& s, u64 line) {
  next_non_whitespace(json, c);
  std::string cs = std::string(1, c);
  std::string ss = std::string(1, s);
  std::string expectedMsg = "Error (line " + std::to_string(line) + "): expected '" + ss + "' but got '" + cs + "'.";
  panic_if(c != s, expectedMsg);
}

void next(std::ifstream& json, char& c) {
  panic_if(json.eof(), "Error: reached end of file.");
  json.get(c);
}

bool is_number(char c) {
  return c == '.' || (c >= '0' && c <= '9');
}

bool is_start_of_number(char c) {
  return c == '-' || is_number(c);
}

f64 parse_number(std::ifstream& json) {
  std::string number;
  number.reserve(30);
  char current;
  next_non_whitespace(json, current);
  while (is_start_of_number(current)) {
    number += current;
    next(json, current);
  }

  panic_if(current != ',' && current != '}', "Error: expected ',', except we got '" + std::string(1, current) + "'");

  return std::stod(number);
}

std::string parse_string(std::ifstream& json) {
  std::string s;
  char current;
  next(json, current);
  while (current != '"') {
    s += current;
    next(json, current);
  }
  return s;
}

void parse_pair_kv(std::ifstream& json, const std::string& expectedKey, f64& value) {
  char current;

  check_next_non_whitespace(json, current, '"', __LINE__);

  std::string key = parse_string(json);
  panic_if(key != expectedKey, "Error: expected " + expectedKey + " but got " + key);

  check_next_non_whitespace(json, current, ':', __LINE__);

  value = parse_number(json);
}

pair parse_pair(std::ifstream& json) {
  pair object;

  char current;
  check_next_non_whitespace(json, current, '{', __LINE__);

  std::string key1 = "x0";
  std::string key2 = "y0";
  std::string key3 = "x1";
  std::string key4 = "y1";

  f64 v1, v2, v3, v4;

  parse_pair_kv(json, key1, v1);
  // check_next_non_whitespace(json, current, ',', __LINE__);

  parse_pair_kv(json, key2, v2);
  // check_next_non_whitespace(json, current, ',', __LINE__);

  parse_pair_kv(json, key3, v3);
  // check_next_non_whitespace(json, current, ',', __LINE__);

  parse_pair_kv(json, key4, v4);
  // check_next_non_whitespace(json, current, '}', __LINE__);

  object.x0 = v1;
  object.y0 = v2;
  object.x1 = v3;
  object.y1 = v4;
  return object;
}

std::vector<pair> parse_pair_array(std::ifstream& json) {
  std::vector<pair> ret;
  char current = 0;

  while (current != ']') {
    ret.push_back(parse_pair(json));
    next_non_whitespace(json, current); // ,
  }
  return ret;
}

std::vector<pair> parse_pairs_json(std::ifstream& json) {
  char c;
  next_non_whitespace(json, c);
  std::string fullMsg = "Error: expected '{' when parsing pairs object, got '" + std::string(1, c) + "'";
  panic_if(c != '{', fullMsg);

  next_non_whitespace(json, c);
  panic_if(c != '"', "Error: expected '\"' when parsing pairs object, got" + std::string(1, c));

  std::string object_name = parse_string(json);
  panic_if(object_name != "pairs", "Error: expected \"pairs\", got \"" + object_name + "\"");
  check_next_non_whitespace(json, c, ':', __LINE__);
  check_next_non_whitespace(json, c, '[', __LINE__);

  return parse_pair_array(json);
}

f64 parse_and_compute(std::ifstream& json) {
  f64 avg = 0.0;

  std::vector<pair> pairs = parse_pairs_json(json);

  for (u64 i{}; i < pairs.size(); i++) {
    lonlat a = { .lon = pairs[i].x0, .lat = pairs[i].y0 };
    lonlat b = { .lon = pairs[i].x1, .lat = pairs[i].y1 };

    avg += reference_haversine(a, b, EARTH_RADIUS);
  }

  return avg / pairs.size();
}

int main(int argc, char ** argv) {
  if (argc != 2 && argc != 3) {
    std::cerr << "Usage: ./parse <json> <answer binary>\n";
    return -1;
  }

  std::string json_name = argv[1];
  std::ifstream json_file;
  json_file.open(json_name);

  if (!json_file.is_open()) {
    std::cerr  << "Error: could not open " << json_name << "\n";
    return -1;
  }

  f64 expected = parse_and_compute(json_file);
  if (argc == 3) {
    std::ifstream binary;
    binary.open(argv[2]);
    if (!binary.is_open()) {
      print_error("had a problem opening: " + std::string(argv[2]));
      exit(-1);
    }
    f64 actual = 0.0;
    char * f = reinterpret_cast<char *>(&actual);
    binary.read(f, sizeof(actual));

    if (std::fabs(expected - actual) < 0.001) {
      std::ostringstream emsg;
      emsg << "Expected: " << expected;
      print_success(emsg.str());

      std::ostringstream eact;
      eact << "Actual: " << actual;
      print_success(eact.str());
    } else {
      std::ostringstream emsg;
      emsg << "Expected: " << expected;
      print_error(emsg.str());

      std::ostringstream eact;
      eact << "Actual: " << actual;
      print_error(eact.str());
    }
  } else {
    std::cout << "Expected: " << expected << "\n";
  }
}
