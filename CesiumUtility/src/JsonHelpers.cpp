#include <CesiumUtility/JsonHelpers.h>
#include <CesiumUtility/JsonValue.h>

#include <glm/ext/matrix_double4x4.hpp>
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace CesiumUtility {

std::optional<double> JsonHelpers::getScalarProperty(
    const rapidjson::Value& tileJson,
    const std::string& key) {
  const auto it = tileJson.FindMember(key.c_str());
  if (it == tileJson.MemberEnd() || !it->value.IsNumber()) {
    return std::nullopt;
  }

  return it->value.GetDouble();
}

std::optional<glm::dmat4x4> JsonHelpers::getTransformProperty(
    const rapidjson::Value& tileJson,
    const std::string& key) {
  const auto it = tileJson.FindMember(key.c_str());
  if (it == tileJson.MemberEnd() || !it->value.IsArray() ||
      it->value.Size() < 16) {
    return std::nullopt;
  }

  const auto& a = it->value.GetArray();

  for (rapidjson::SizeType i = 0; i < 16; ++i) {
    if (!a[i].IsNumber()) {
      return std::nullopt;
    }
  }

  return glm::dmat4(
      glm::dvec4(
          a[0].GetDouble(),
          a[1].GetDouble(),
          a[2].GetDouble(),
          a[3].GetDouble()),
      glm::dvec4(
          a[4].GetDouble(),
          a[5].GetDouble(),
          a[6].GetDouble(),
          a[7].GetDouble()),
      glm::dvec4(
          a[8].GetDouble(),
          a[9].GetDouble(),
          a[10].GetDouble(),
          a[11].GetDouble()),
      glm::dvec4(
          a[12].GetDouble(),
          a[13].GetDouble(),
          a[14].GetDouble(),
          a[15].GetDouble()));
}

std::optional<std::vector<double>> JsonHelpers::getDoubles(
    const rapidjson::Value& json,
    int32_t expectedSize,
    const std::string& key) {
  const auto it = json.FindMember(key.c_str());
  if (it == json.MemberEnd()) {
    return std::nullopt;
  }
  const auto& value = it->value;
  return JsonHelpers::getDoubles(value, expectedSize);
}

std::optional<std::vector<double>>
JsonHelpers::getDoubles(const rapidjson::Value& json, int32_t expectedSize) {
  if (!json.IsArray()) {
    return std::nullopt;
  }
  if (expectedSize >= 0 &&
      json.Size() != static_cast<rapidjson::SizeType>(expectedSize)) {
    return std::nullopt;
  }
  const auto& a = json.GetArray();
  std::vector<double> result;
  result.reserve(json.Size());
  for (rapidjson::SizeType i = 0; i < json.Size(); ++i) {
    if (!a[i].IsNumber()) {
      return std::nullopt;
    }
    result.emplace_back(a[i].GetDouble());
  }
  return result;
}

std::string JsonHelpers::getStringOrDefault(
    const rapidjson::Value& json,
    const std::string& key,
    const std::string& defaultValue) {
  const auto it = json.FindMember(key.c_str());
  return it == json.MemberEnd()
             ? defaultValue
             : JsonHelpers::getStringOrDefault(it->value, defaultValue);
}

std::string JsonHelpers::getStringOrDefault(
    const rapidjson::Value& json,
    const std::string& defaultValue) {
  if (json.IsString()) {
    return json.GetString();
  }
  return defaultValue;
}

double JsonHelpers::getDoubleOrDefault(
    const rapidjson::Value& json,
    const std::string& key,
    double defaultValue) {
  const auto it = json.FindMember(key.c_str());
  return it == json.MemberEnd()
             ? defaultValue
             : JsonHelpers::getDoubleOrDefault(it->value, defaultValue);
}

double JsonHelpers::getDoubleOrDefault(
    const rapidjson::Value& json,
    double defaultValue) {
  if (json.IsNumber()) {
    return json.GetDouble();
  }
  return defaultValue;
}

uint32_t JsonHelpers::getUint32OrDefault(
    const rapidjson::Value& json,
    const std::string& key,
    uint32_t defaultValue) {
  const auto it = json.FindMember(key.c_str());
  return it == json.MemberEnd()
             ? defaultValue
             : JsonHelpers::getUint32OrDefault(it->value, defaultValue);
}

uint32_t JsonHelpers::getUint32OrDefault(
    const rapidjson::Value& json,
    uint32_t defaultValue) {
  if (json.IsUint()) {
    return json.GetUint();
  }
  return defaultValue;
}

int32_t JsonHelpers::getInt32OrDefault(
    const rapidjson::Value& json,
    const std::string& key,
    int32_t defaultValue) {
  const auto it = json.FindMember(key.c_str());
  return it == json.MemberEnd()
             ? defaultValue
             : JsonHelpers::getInt32OrDefault(it->value, defaultValue);
}

int32_t JsonHelpers::getInt32OrDefault(
    const rapidjson::Value& json,
    int32_t defaultValue) {
  if (json.IsInt()) {
    return json.GetInt();
  }
  return defaultValue;
}

uint64_t JsonHelpers::getUint64OrDefault(
    const rapidjson::Value& json,
    const std::string& key,
    uint64_t defaultValue) {
  const auto it = json.FindMember(key.c_str());
  return it == json.MemberEnd()
             ? defaultValue
             : JsonHelpers::getUint64OrDefault(it->value, defaultValue);
}

uint64_t JsonHelpers::getUint64OrDefault(
    const rapidjson::Value& json,
    uint64_t defaultValue) {
  if (json.IsUint()) {
    return json.GetUint();
  }
  return defaultValue;
}

int64_t JsonHelpers::getInt64OrDefault(
    const rapidjson::Value& json,
    const std::string& key,
    int64_t defaultValue) {
  const auto it = json.FindMember(key.c_str());
  return it == json.MemberEnd()
             ? defaultValue
             : JsonHelpers::getInt64OrDefault(it->value, defaultValue);
}

int64_t JsonHelpers::getInt64OrDefault(
    const rapidjson::Value& json,
    int64_t defaultValue) {
  if (json.IsInt()) {
    return json.GetInt();
  }
  return defaultValue;
}

bool JsonHelpers::getBoolOrDefault(
    const rapidjson::Value& json,
    const std::string& key,
    bool defaultValue) {
  const auto it = json.FindMember(key.c_str());
  return it == json.MemberEnd()
             ? defaultValue
             : JsonHelpers::getBoolOrDefault(it->value, defaultValue);
}

bool JsonHelpers::getBoolOrDefault(
    const rapidjson::Value& json,
    bool defaultValue) {
  if (json.IsBool()) {
    return json.GetBool();
  }
  return defaultValue;
}

std::vector<std::string>
JsonHelpers::getStrings(const rapidjson::Value& json, const std::string& key) {
  std::vector<std::string> result;

  const auto it = json.FindMember(key.c_str());
  if (it != json.MemberEnd() && it->value.IsArray()) {
    const auto& valueJson = it->value;

    result.reserve(valueJson.Size());

    for (rapidjson::SizeType i = 0; i < valueJson.Size(); ++i) {
      const auto& element = valueJson[i];
      if (element.IsString()) {
        result.emplace_back(element.GetString());
      }
    }
  }

  return result;
}

std::vector<int64_t>
JsonHelpers::getInt64s(const rapidjson::Value& json, const std::string& key) {
  std::vector<int64_t> result;

  const auto it = json.FindMember(key.c_str());
  if (it != json.MemberEnd() && it->value.IsArray()) {
    const auto& valueJson = it->value;

    result.reserve(valueJson.Size());

    for (rapidjson::SizeType i = 0; i < valueJson.Size(); ++i) {
      const auto& element = valueJson[i];
      if (element.IsInt64()) {
        result.emplace_back(element.GetInt64());
      }
    }
  }

  return result;
}

JsonValue JsonHelpers::toJsonValue(const rapidjson::Value& json) {
  if (json.IsBool()) {
    return JsonValue(json.GetBool());
  } else if (json.IsDouble()) {
    return JsonValue(json.GetDouble());
  } else if (json.IsFloat()) {
    return JsonValue(json.GetFloat());
  } else if (json.IsInt64()) {
    return JsonValue(json.GetInt64());
  } else if (json.IsInt()) {
    return JsonValue(json.GetInt());
  } else if (json.IsNull()) {
    return JsonValue(nullptr);
  } else if (json.IsString()) {
    return JsonValue(json.GetString());
  } else if (json.IsUint64()) {
    return JsonValue(json.GetUint64());
  } else if (json.IsUint()) {
    return JsonValue(json.GetUint());
  } else if (json.IsArray()) {
    JsonValue::Array arr;
    for (auto& value : json.GetArray()) {
      arr.emplace_back(JsonHelpers::toJsonValue(value));
    }
    return arr;
  } else if (json.IsObject()) {
    JsonValue::Object obj;
    for (auto& [key, value] : json.GetObject()) {
      obj.emplace(key.GetString(), JsonHelpers::toJsonValue(value));
    }
    return obj;
  }

  return JsonValue::Null();
}
} // namespace CesiumUtility
