/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * E133PDU.cpp
 * The E133PDU
 * Copyright (C) 2011 Simon Newton
 */


#include <string.h>
#include <string>
#include "ola/Logging.h"
#include "ola/base/Array.h"
#include "ola/network/NetworkUtils.h"
#include "ola/strings/Utils.h"
#include "libs/acn/E133PDU.h"

namespace ola {
namespace acn {

using ola::io::OutputStream;
using ola::network::HostToNetwork;
using std::string;

/*
 * Size of the header portion.
 */
unsigned int E133PDU::HeaderSize() const {
  return sizeof(E133Header::e133_pdu_header);
}


/*
 * Size of the data portion
 */
unsigned int E133PDU::DataSize() const {
  return m_pdu ? m_pdu->Size() : 0;
}


/*
 * Pack the header portion.
 */
bool E133PDU::PackHeader(uint8_t *data, unsigned int *length) const {
  unsigned int header_size = HeaderSize();

  if (*length < header_size) {
    OLA_WARN << "E133PDU::PackHeader: buffer too small, got " << *length
             << " required " << header_size;
    *length = 0;
    return false;
  }

  E133Header::e133_pdu_header header;
  strings::CopyToFixedLengthBuffer(m_header.Source(), header.source,
                                   arraysize(header.source));
  header.sequence = HostToNetwork(m_header.Sequence());
  header.endpoint = HostToNetwork(m_header.Endpoint());
  header.reserved = 0;
  *length = sizeof(E133Header::e133_pdu_header);
  memcpy(data, &header, *length);
  return true;
}


/*
 * Pack the data portion.
 */
bool E133PDU::PackData(uint8_t *data, unsigned int *length) const {
  if (m_pdu)
    return m_pdu->Pack(data, length);
  *length = 0;
  return true;
}


/*
 * Pack the header into a buffer.
 */
void E133PDU::PackHeader(OutputStream *stream) const {
  E133Header::e133_pdu_header header;
  strings::CopyToFixedLengthBuffer(m_header.Source(), header.source,
                                   arraysize(header.source));
  header.sequence = HostToNetwork(m_header.Sequence());
  header.endpoint = HostToNetwork(m_header.Endpoint());
  header.reserved = 0;
  stream->Write(reinterpret_cast<uint8_t*>(&header),
                sizeof(E133Header::e133_pdu_header));
}


/*
 * Pack the data into a buffer
 */
void E133PDU::PackData(OutputStream *stream) const {
  if (m_pdu)
    m_pdu->Write(stream);
}


void E133PDU::PrependPDU(ola::io::IOStack *stack, uint32_t vector,
                         const string &source_name, uint32_t sequence_number,
                         uint16_t endpoint_id) {
  E133Header::e133_pdu_header header;
  strings::CopyToFixedLengthBuffer(source_name, header.source,
                                   arraysize(header.source));
  header.sequence = HostToNetwork(sequence_number);
  header.endpoint = HostToNetwork(endpoint_id);
  header.reserved = 0;
  stack->Write(reinterpret_cast<uint8_t*>(&header),
               sizeof(E133Header::e133_pdu_header));

  vector = HostToNetwork(vector);
  stack->Write(reinterpret_cast<uint8_t*>(&vector), sizeof(vector));
  PrependFlagsAndLength(stack);
}
}  // namespace acn
}  // namespace ola