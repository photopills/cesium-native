// This file was generated by generate-classes.
// DO NOT EDIT THIS FILE!
#pragma once

#include <Cesium3DTiles/BufferView.h>
#include <CesiumJsonReader/ExtensibleObjectJsonHandler.h>
#include <CesiumJsonReader/IntegerJsonHandler.h>
#include <CesiumJsonReader/StringJsonHandler.h>

namespace CesiumJsonReader {
class JsonReaderOptions;
} // namespace CesiumJsonReader

namespace Cesium3DTilesReader {
class BufferViewJsonHandler
    : public CesiumJsonReader::ExtensibleObjectJsonHandler {
public:
  using ValueType = Cesium3DTiles::BufferView;

  explicit BufferViewJsonHandler(
      const CesiumJsonReader::JsonReaderOptions& options) noexcept;
  void reset(IJsonHandler* pParentHandler, Cesium3DTiles::BufferView* pObject);

  IJsonHandler* readObjectKey(const std::string_view& str) override;

protected:
  IJsonHandler* readObjectKeyBufferView(
      const std::string& objectType,
      const std::string_view& str,
      Cesium3DTiles::BufferView& o);

private:
  Cesium3DTiles::BufferView* _pObject = nullptr;
  CesiumJsonReader::IntegerJsonHandler<int64_t> _buffer;
  CesiumJsonReader::IntegerJsonHandler<int64_t> _byteOffset;
  CesiumJsonReader::IntegerJsonHandler<int64_t> _byteLength;
  CesiumJsonReader::StringJsonHandler _name;
};
} // namespace Cesium3DTilesReader
