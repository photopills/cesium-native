// This file was generated by generate-classes.
// DO NOT EDIT THIS FILE!
#pragma once

#include <Cesium3DTiles/ClassProperty.h>
#include <CesiumJsonReader/BoolJsonHandler.h>
#include <CesiumJsonReader/ExtensibleObjectJsonHandler.h>
#include <CesiumJsonReader/IntegerJsonHandler.h>
#include <CesiumJsonReader/JsonObjectJsonHandler.h>
#include <CesiumJsonReader/StringJsonHandler.h>

namespace CesiumJsonReader {
class JsonReaderOptions;
} // namespace CesiumJsonReader

namespace Cesium3DTilesReader {
class ClassPropertyJsonHandler
    : public CesiumJsonReader::ExtensibleObjectJsonHandler {
public:
  using ValueType = Cesium3DTiles::ClassProperty;

  explicit ClassPropertyJsonHandler(
      const CesiumJsonReader::JsonReaderOptions& options) noexcept;
  void
  reset(IJsonHandler* pParentHandler, Cesium3DTiles::ClassProperty* pObject);

  IJsonHandler* readObjectKey(const std::string_view& str) override;

protected:
  IJsonHandler* readObjectKeyClassProperty(
      const std::string& objectType,
      const std::string_view& str,
      Cesium3DTiles::ClassProperty& o);

private:
  Cesium3DTiles::ClassProperty* _pObject = nullptr;
  CesiumJsonReader::StringJsonHandler _name;
  CesiumJsonReader::StringJsonHandler _description;
  CesiumJsonReader::StringJsonHandler _type;
  CesiumJsonReader::StringJsonHandler _componentType;
  CesiumJsonReader::StringJsonHandler _enumType;
  CesiumJsonReader::BoolJsonHandler _array;
  CesiumJsonReader::IntegerJsonHandler<int64_t> _count;
  CesiumJsonReader::BoolJsonHandler _normalized;
  CesiumJsonReader::JsonObjectJsonHandler _offset;
  CesiumJsonReader::JsonObjectJsonHandler _scale;
  CesiumJsonReader::JsonObjectJsonHandler _max;
  CesiumJsonReader::JsonObjectJsonHandler _min;
  CesiumJsonReader::BoolJsonHandler _required;
  CesiumJsonReader::JsonObjectJsonHandler _noData;
  CesiumJsonReader::JsonObjectJsonHandler _defaultProperty;
  CesiumJsonReader::StringJsonHandler _semantic;
};
} // namespace Cesium3DTilesReader
