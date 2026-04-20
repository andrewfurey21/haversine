
// Simple C++ json parsing, not compliant.

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <alloca.h>

#include "types.hpp"
#include "profiler.hpp"


struct Buffer {
  u8 * data;
  u64 capacity;
  Buffer(u64 capacity) : capacity(capacity), data(NULL) {
    if (capacity > 0) {
      data = (u8 *)malloc(capacity * sizeof(u8));
    }
  }
  Buffer() : Buffer(0) {}
  Buffer(const Buffer& other) : capacity(other.capacity) {
    if (capacity > 0) {
      data = (u8 *)malloc(sizeof(u8) * capacity);
      memcpy(data, other.data, capacity);
    } else {
      data = NULL;
    }
  }
  Buffer& operator=(const Buffer& other) {
    if (this != &other) {
      if (this->capacity > 0) {
        free(this->data);
      }

      this->capacity = other.capacity;
      if (this->capacity > 0) {
        this->data = (u8 *)malloc(sizeof(u8) * this->capacity);
        memcpy(this->data, other.data, this->capacity);
      } else {
        this->data = NULL;
      }
    }
    return *this;
  }
  Buffer& operator=(Buffer&& other) {
    if (this != &other) {
      if (this->capacity > 0) {
        free(this->data);
      }

      this->capacity = other.capacity;
      this->data = other.data;
      other.data = NULL;
    }
    return *this;
  }
  Buffer(Buffer&& other) : capacity(other.capacity) {
    data = other.data;
    other.data = NULL;
  }

  void destroy() {
    free(data);
    data = NULL;
    capacity = 0;
  }

  ~Buffer() {
    destroy();
  }
};

enum class JSONTokenType : u8 {
  UNSPECIFIED = 0,
  OPEN_CURLY = '{',
  CLOSED_CURLY = '}',
  OPEN_BRACKET = '[',
  CLOSED_BRACKET = ']',

  FALSE = 'f',
  TRUE = 't',
  NUMBER = 'N',
  STRING = 'S',
  JSONNULL = '-',

  COMMA = ',',
  COLON = ':',

  ERROR = 'E',
  END_OF_FILE = '!',
};

struct JSONToken {
  JSONTokenType type;
  Buffer buffer;
  JSONToken() : type(JSONTokenType::UNSPECIFIED), buffer() {}
};


inline b8 is_whitespace(u8 character) {
  return (character == '\t' || character == ' ' ||
          character == '\r' || character == '\n');
}

inline u8 read_char(FILE * f, u8 * character) {
  u64 advanced = fread(character, sizeof(u8), 1, f);
  assert(advanced == sizeof(u8) || feof(f));
  return *character;
}

inline b8 match_lookahead(FILE * f, const char * match) {
  for (u64 i{}; i < strlen(match); i++) {
    u8 current;
    read_char(f, &current);
    if (match[i] != current) return false;
  }
  return true;
}

inline void copy_string(JSONToken& token, FILE * f, const u64 alignment = 16) {
  // NOTE: Stack has to grow downward.
  u8 * string_stack = (u8 *)alloca(sizeof(u8));
  u64 count = 0;

  // TODO: recognize escaped characters.
  while (read_char(f, string_stack) != '"' && !feof(f)) {
    count++;

    // NOTE: no check for overflow. Alignment also matters here.
    string_stack = (u8 *)alloca(sizeof(u8));
  }

  if (feof(f)) {
    token.type = JSONTokenType::ERROR; // TODO: add error message to buffer.
    return;
  }

  if (count == 0) return;

  token.buffer = Buffer(count);

  for (u64 buffer_index = 1; buffer_index <= count; buffer_index++) {
    u64 stack_index = buffer_index * alignment;
    token.buffer.data[count - buffer_index] = string_stack[stack_index];
  }
}

// NOTE: not standard json.
inline b8 is_numeric(u8 c) {
  return c == '-' || ('0' <= c && c <= '9') || c == '.';
}

// NOTE: not standard json.
inline void check_number(JSONToken& token) {
  b8 correct = true;

  u8 num_points = 0;
  for (u64 i = 0; i < token.buffer.capacity; i++) {
    u8 c = token.buffer.data[i];
    if (!is_numeric(c) || num_points >= 2 || (c == '-' && i != 0))  {
      correct = false;
      break;
    }

    if (c == '.') num_points++;
  }

  if (!correct) {
    token.buffer.destroy();
    token.type = JSONTokenType::ERROR;
  }
}

inline void read_number(JSONToken& token, FILE * f, u8 start, const u64 alignment = 16) {
    u8 * number_stack = (u8 *)alloca(sizeof(u8));
    u64 count = 1;

    u32 last = ftell(f);
    while (is_numeric(read_char(f, number_stack)) && !feof(f)) {
      count++;
      number_stack = (u8 *)alloca(sizeof(u8));
      last = ftell(f);
    }
    fseek(f, last, SEEK_SET);

    if (feof(f)) {
      token.type = JSONTokenType::ERROR;
      return;
    }

    token.buffer.capacity = count;
    token.buffer.data = (u8 *)malloc(count * sizeof(u8));

    for (u64 buffer_index = 1; buffer_index < count; buffer_index++) {
      u64 stack_index = buffer_index * alignment;
      token.buffer.data[count - buffer_index] = number_stack[stack_index];
    }
    token.buffer.data[0] = start;

    check_number(token);
}

inline JSONToken get_next_token(FILE * file) {
  JSONToken token;

  u8 current_character = ' ';
  while (!feof(file) && is_whitespace(current_character)) {
    read_char(file, &current_character);
  }

  if (feof(file)) {
    token.type = JSONTokenType::END_OF_FILE;
    return token;
  }

  switch(current_character) {

    case '{': {
        token.type = JSONTokenType::OPEN_CURLY;
      } break;
    case '}': {
        token.type = JSONTokenType::CLOSED_CURLY;
      } break;

    case '[': {
        token.type = JSONTokenType::OPEN_BRACKET;
      } break;
    case ']': {
        token.type = JSONTokenType::CLOSED_BRACKET;
      } break;

    case ',': {
        token.type = JSONTokenType::COMMA;
      } break;
    case ':': {
        token.type = JSONTokenType::COLON;
      } break;

    case 'f': {
        if (match_lookahead(file, "alse")) token.type = JSONTokenType::FALSE;
        else token.type = JSONTokenType::ERROR;
      } break;
    case 't': {
        if (match_lookahead(file, "rue")) token.type = JSONTokenType::TRUE;
        else token.type = JSONTokenType::ERROR;
      } break;
    case 'n': {
        if (match_lookahead(file, "ull")) token.type = JSONTokenType::JSONNULL;
        else token.type = JSONTokenType::ERROR;
      } break;

    case '"': {
        token.type = JSONTokenType::STRING;
        copy_string(token, file);
      } break;

    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': {
        token.type = JSONTokenType::NUMBER;
        read_number(token, file, current_character);
      } break;

    default: {
      token.type = JSONTokenType::ERROR;
    } break;
  }

  return token;
}

struct JSONElement {
  JSONToken key;
  JSONElement * value; // speed up with union with f64?
  JSONElement * next;

  JSONElement() : key(), value(NULL), next(NULL) {}
};

inline void destroy_json(JSONElement * json) {
  if (json->key.buffer.data != NULL) json->key.buffer.destroy();
  if (json->value != NULL) destroy_json(json->value);
  if (json->next != NULL) destroy_json(json->next);

  delete json;
}

inline JSONElement * parse_literal(const JSONToken& token) {
  if (token.type != JSONTokenType::FALSE  &&
      token.type != JSONTokenType::TRUE   &&
      token.type != JSONTokenType::NUMBER &&
      token.type != JSONTokenType::STRING &&
      token.type != JSONTokenType::JSONNULL) {
    printf("Error: could not parse literal.\n");
    return NULL;
  }

  JSONElement * literal = new JSONElement();
  literal->key = token; // copied :(

  return literal;
}

JSONElement * parse_json(FILE * file);
// TODO: proper error and clean up function.

inline JSONElement * parse_object(FILE * file) {
  PROFILE_FUNCTION;
  //     previous <-------
  //                     |
  // { "key": json, "key": json, ... }

  JSONElement * previous = NULL;
  JSONElement * object = new JSONElement();
  JSONElement * head = object;

  enum class State {
    KEY, COLON, JSON, COMMA
  };

  State state = State::KEY;
  // [ string, colon, json, comma ]
  JSONToken current_token = {};
  while (current_token.type != JSONTokenType::END_OF_FILE &&
         current_token.type != JSONTokenType::ERROR) {

    switch (state) {
      case State::KEY: {
          current_token = get_next_token(file);
          if (previous != NULL) {
            object = new JSONElement();
          }

          if (current_token.type != JSONTokenType::STRING) {
            printf("parse_object() Error: expected string as key, got %d.\n", (int)current_token.type);
            return NULL;
          }

          object->key = current_token;
          state = State::COLON;
        } break;

      case State::COLON: {
          current_token = get_next_token(file);
          if (current_token.type != JSONTokenType::COLON) {
            printf("parse_object() Error: expected ':' but got %d.\n", (int)current_token.type);
            return NULL;
          }
          state = State::JSON;
        } break;

      case State::JSON: {
          object->value = parse_json(file);
          state = State::COMMA;
        } break;

      case State::COMMA: {
          current_token = get_next_token(file);
          if (current_token.type == JSONTokenType::CLOSED_CURLY) {
            if (previous != NULL) previous->next = object;
            return head;
          }

          if (current_token.type != JSONTokenType::COMMA) {
            printf("parse_object() Error: expected ',' but got %d.\n", (int)current_token.type);
            return NULL;
          }
          state = State::KEY;

          if (previous != NULL) {
            previous->next = object;
          }
          previous = object;
        } break;
    }
  }

  if (current_token.type == JSONTokenType::END_OF_FILE) {
    printf("parse_object() Error: missing '}'\n");
    return NULL;
  }

  if (current_token.type == JSONTokenType::ERROR) {
    printf("parse_object() Error: bad token when parsing object.\n");
    return NULL;
  }

  // shouldn't have gotten here.
  return NULL;
}

// very similar to parse_object, except no keys.
inline JSONElement * parse_list(FILE * file) {
  PROFILE_FUNCTION;
  // [ json, json, ... ]
  JSONElement * previous = NULL;
  JSONElement * list = new JSONElement();
  JSONElement * head = list;

  enum class State { JSON, COMMA };

  State state = State::JSON;

  JSONToken current_token;
  while (current_token.type != JSONTokenType::END_OF_FILE &&
         current_token.type != JSONTokenType::ERROR) {
    switch (state) {
      case State::JSON: {
          if (current_token.type == JSONTokenType::CLOSED_BRACKET) {
            return head;
          }

          if (previous != NULL) {
            list = new JSONElement();
          }

          list->value = parse_json(file);

          if (previous != NULL) {
            previous->next = list;
          }

          previous = list;
        } break;
      case State::COMMA: {
          if (current_token.type != JSONTokenType::COMMA) {
            printf("Error: expected ',' seperating objects in an array, got %c\n", (int)current_token.type);
            return NULL;
          }
        } break;
    }
    current_token = get_next_token(file);
  }

  if (current_token.type == JSONTokenType::END_OF_FILE) {
    printf("Error: missing ']'\n");
    return NULL;
  }

  if (current_token.type == JSONTokenType::ERROR) {
    printf("Error: bad token when parsing list.\n");
    return NULL;
  }

  // shouldn't have gotten here.
  return NULL;
}

// parses a literal, array or object.
inline JSONElement * parse_json(FILE * file) {
  PROFILE_FUNCTION;
  // PROFILE_BLOCK("parse_json");
  // Block parse_json_block = Block("parse_json", 5);

  JSONElement * json = NULL;

  JSONToken current_token = get_next_token(file);
  switch (current_token.type) {
    case JSONTokenType::OPEN_CURLY: {
        // parse object
        json = parse_object(file);
      } break;

    case JSONTokenType::OPEN_BRACKET: {
        // parse list.
        json = parse_list(file);
      } break;

    case JSONTokenType::FALSE:
    case JSONTokenType::TRUE:
    case JSONTokenType::NUMBER:
    case JSONTokenType::STRING:
    case JSONTokenType::JSONNULL: {
        json = parse_literal(current_token);
      } break;

    default: {
       printf("Error: token type = %c\n", (u8)current_token.type);
       return NULL;
     } break;
  }
  return json;
}

inline b8 string_equal(const Buffer& buffer, const char * str) {
  u64 len = strlen(str);
  if (buffer.capacity != len) {
    return false;
  }

  for (u64 i = 0; i < len; i++) {
    if (buffer.data[i] != str[i]) return false;
  }

  return true;
}

inline JSONElement * get_object_value(JSONElement * const json, const char * key) {
  JSONElement * current = json;
  while (current != NULL) {
    if (current->key.type != JSONTokenType::STRING) {
      // all keys should be strings.
      printf("get_object_value() Error: This is not an object.\n");
      return NULL;
    }

    if (string_equal(current->key.buffer, key)) {
      return current->value;
    }
    current = current->next;
  }

  printf("get_object_value() Error: Could not find object.\n");
  return NULL;
}

inline f64 convert_to_number(JSONElement * json) {
  if (json->key.type != JSONTokenType::NUMBER) {
    printf("convert_to_number() Error: not a number.\n");
    return 0;
  }

  // number should have been parsed with check_number
  Buffer& number = json->key.buffer;

  f64 value = 0;
  f64 sign = 1;

  b8 fraction = false;
  u64 number_places = 0;

  for (u64 i = 0; i < number.capacity; i++) {
    u8 digit = number.data[i] - '0';
    if (number.data[i] == '-') {
      sign = -1;
    } else if (number.data[i] == '.') {
      number_places = 1;
      fraction = true;
    }else if (!fraction) {
      value = value * 10 + digit;
    } else {
      value += (f64)digit / pow(10.0, number_places);
      number_places++;
    }
  }
  return value * sign;
}
