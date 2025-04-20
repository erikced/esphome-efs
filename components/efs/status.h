#pragma once

enum Status {
  Ok,
  BufferNotAligned,
  StartNotFound,
  WriteOverflow,
  InvalidObisCode,
  ParsingFailed,
  HeaderTooLong,
  ObjectTooLong,
  TooManyObjects,
  InvalidCrc,
  CrcCheckFailed,
};
