// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_TYPED_ARRAYS_DOM_ARRAY_BUFFER_VIEW_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_TYPED_ARRAYS_DOM_ARRAY_BUFFER_VIEW_H_

#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/core/typed_arrays/dom_array_buffer.h"
#include "third_party/blink/renderer/core/typed_arrays/dom_shared_array_buffer.h"
#include "third_party/blink/renderer/platform/bindings/script_wrappable.h"

namespace blink {

class CORE_EXPORT DOMArrayBufferView : public ScriptWrappable {
  DEFINE_WRAPPERTYPEINFO();

 public:
  enum ViewType {
    kTypeInt8,
    kTypeUint8,
    kTypeUint8Clamped,
    kTypeInt16,
    kTypeUint16,
    kTypeInt32,
    kTypeUint32,
    kTypeFloat32,
    kTypeFloat64,
    kTypeBigInt64,
    kTypeBigUint64,
    kTypeDataView
  };

  ~DOMArrayBufferView() override = default;

  DOMArrayBuffer* buffer() const {
    DCHECK(!IsShared());
    DCHECK(dom_array_buffer_);
    return static_cast<DOMArrayBuffer*>(dom_array_buffer_.Get());
  }

  DOMSharedArrayBuffer* BufferShared() const {
    DCHECK(IsShared());
    DCHECK(dom_array_buffer_);
    return static_cast<DOMSharedArrayBuffer*>(dom_array_buffer_.Get());
  }

  DOMArrayBufferBase* BufferBase() const {
    if (IsShared())
      return BufferShared();

    return buffer();
  }

  virtual ViewType GetType() const = 0;

  const char* TypeName() {
    switch (GetType()) {
      case kTypeInt8:
        return "Int8";
        break;
      case kTypeUint8:
        return "UInt8";
        break;
      case kTypeUint8Clamped:
        return "UInt8Clamped";
        break;
      case kTypeInt16:
        return "Int16";
        break;
      case kTypeUint16:
        return "UInt16";
        break;
      case kTypeInt32:
        return "Int32";
        break;
      case kTypeUint32:
        return "Uint32";
        break;
      case kTypeBigInt64:
        return "BigInt64";
        break;
      case kTypeBigUint64:
        return "BigUint64";
        break;
      case kTypeFloat32:
        return "Float32";
        break;
      case kTypeFloat64:
        return "Float64";
        break;
      case kTypeDataView:
        return "DataView";
        break;
    }
  }

  void* BaseAddress() const {
    DCHECK(!IsShared());
    return BaseAddressMaybeShared();
  }

  size_t byteOffsetAsSizeT() const {
    return !IsDetached() ? raw_byte_offset_ : 0;
  }
  // This function is deprecated and should not be used. Use {byteOffsetAsSizeT}
  // instead.
  unsigned deprecatedByteOffsetAsUnsigned() const {
    return base::checked_cast<unsigned>(byteOffsetAsSizeT());
  }
  virtual size_t byteLengthAsSizeT() const = 0;
  // This function is deprecated and should not be used. Use {byteLengthAsSizeT}
  // instead.
  unsigned deprecatedByteLengthAsUnsigned() const {
    return base::checked_cast<unsigned>(byteLengthAsSizeT());
  }
  virtual unsigned TypeSize() const = 0;
  bool IsShared() const { return dom_array_buffer_->IsShared(); }

  void* BaseAddressMaybeShared() const {
    return !IsDetached() ? raw_base_address_ : nullptr;
  }

  v8::Local<v8::Value> Wrap(v8::Isolate*,
                            v8::Local<v8::Object> creation_context) override {
    NOTREACHED();
    return v8::Local<v8::Object>();
  }

  void Trace(Visitor* visitor) const override {
    visitor->Trace(dom_array_buffer_);
    ScriptWrappable::Trace(visitor);
  }

 protected:
  DOMArrayBufferView(DOMArrayBufferBase* dom_array_buffer, size_t byte_offset)
      : raw_byte_offset_(byte_offset), dom_array_buffer_(dom_array_buffer) {
    DCHECK(dom_array_buffer_);
    raw_base_address_ =
        static_cast<char*>(dom_array_buffer_->DataMaybeShared()) + byte_offset;
  }

  bool IsDetached() const { return dom_array_buffer_->IsDetached(); }

 private:
  // The raw_* fields may be stale after Detach. Use getters instead.
  // This is the address of the ArrayBuffer's storage, plus the byte offset.
  void* raw_base_address_;
  size_t raw_byte_offset_;

  mutable Member<DOMArrayBufferBase> dom_array_buffer_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_TYPED_ARRAYS_DOM_ARRAY_BUFFER_VIEW_H_
