//
// Copyright 2010-2014 Ettus Research LLC
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef INCLUDED_UHD_TRANSPORT_VRT_IF_PACKET_HPP
#define INCLUDED_UHD_TRANSPORT_VRT_IF_PACKET_HPP

#include <uhd/config.hpp>
#include <boost/cstdint.hpp>
#include <cstddef> //size_t

namespace uhd{ namespace transport{

namespace vrt{

    //! The maximum number of 32-bit words in the vrlp link layer
    static const size_t num_vrl_words32 = 3;

    //! The maximum number of 32-bit words in a vrt if packet header
    static const size_t max_if_hdr_words32 = 7; //hdr+sid+cid+tsi+tsf

    /*!
     * Definition for fields that can be packed into a vrt if header.
     * The size fields are used for input and output depending upon
     * the operation used (ie the pack or unpack function call).
     */
    struct UHD_API if_packet_info_t
    {
        if_packet_info_t(void);

        //link layer type - always set for pack and unpack
        enum link_type_t
        {
            LINK_TYPE_NONE = 0x0,
            LINK_TYPE_CHDR = 0x1,
            LINK_TYPE_VRLP = 0x2
        } link_type;

        //packet type
        enum packet_type_t
        {
            // VRT language:
            PACKET_TYPE_DATA      = 0x0,
            PACKET_TYPE_IF_EXT    = 0x1,
            PACKET_TYPE_CONTEXT   = 0x2, //extension context: has_sid = true

            // CVITA language:
            //PACKET_TYPE_DATA      = 0x0, // Data
            PACKET_TYPE_FC        = 0x1, // Flow control
            PACKET_TYPE_ACK       = 0x1, // Flow control (ack)
            PACKET_TYPE_CMD       = 0x2, // Command
            PACKET_TYPE_RESP      = 0x3, // Command response
            PACKET_TYPE_ERROR     = 0x3  // Command response: Error (the EOB bit is raised in this case)
        } packet_type;

        //size fields
        size_t num_payload_words32; //required in pack, derived in unpack
        size_t num_payload_bytes;   //required in pack, derived in unpack
        size_t num_header_words32;  //derived in pack, derived in unpack
        size_t num_packet_words32;  //derived in pack, required in unpack

        //header fields
        size_t packet_count;
        //! Asserted for start- or end-of-burst
        bool sob, eob;
        //! This is asserted for command responses that are errors (CHDR only)
        bool error;

        //optional fields
        //! Stream ID (SID). See uhd::sid_t
        bool has_sid; boost::uint32_t sid;
        //! Class ID.
        bool has_cid; boost::uint64_t cid;
        //! Integer timestamp
        bool has_tsi; boost::uint32_t tsi;
        //! Fractional timestamp
        bool has_tsf; boost::uint64_t tsf;
        //! Trailer
        bool has_tlr; boost::uint32_t tlr;
    };

    /*!
     * Pack a vrt header from metadata (big endian format).
     *
     * \section vrt_pack_contract Packing contract
     *
     * \subsection Requirements:
     * - packet_buff points to a valid address space with enough space to write
     *   the entire buffer, regardless of its length. At the very least, it must
     *   be able to hold an entire header.
     * - `if_packet_info` has the following members set to correct values:
     *   - `has_*` members all set accordingly
     *   - For every true `has_*` member, the corresponding variable holds a valid
     *     value (e.g. if `has_sid` is true, `sid` contains a valid SID)
     *   - `num_payload_bytes` and `num_payload_words32` are both set to the correct values
     *
     * \subsection Result:
     * - `packet_buff` now points to a valid header that can be sent over the transport
     *   without further modification
     * - The following members on `if_packet_info` are set:
     *   - `num_header_words32`
     *   - `num_packet_words32`
     *
     * \param packet_buff memory to write the packed vrt header
     * \param if_packet_info the if packet info (read/write)
     */
    UHD_API void if_hdr_pack_be(
        boost::uint32_t *packet_buff,
        if_packet_info_t &if_packet_info
    );

    /*!
     * Unpack a vrt header to metadata (big endian format).
     *
     * \section vrt_unpack_contract Unpacking contract
     *
     * \subsection Requirements
     * - `packet_buff` points to a readable address space with a
     *   CHDR packet, starting at the header. `packet_buff[0]` *must* always
     *   point to a valid first word of the header. This implies that num_packet_words32
     *   must be at least 1.
     * - `if_packet_info` has the following members set to correct values:
     *   - `num_packet_words32`. This means all values `packet_buff[0]`
     *     through `packet_buff[if_packet_info.num_packet_words32-1]` are
     *     readable words from this packet.
     *   - `link_type`
     *
     * \subsection Result
     * - `if_packet_info` now has the following values set to correct values:
     *   - `packet_type`
     *   - `num_payload_bytes`
     *   - `num_payload_words32`
     *   - `num_header_words32`
     *   - `has_*`
     *   - `sob`, `eob`, `error`, `cid`, `sid` (if applicable)
     *   - `tsf`, `tsi` (if applicable)
     *
     * \subsection Exceptions
     * - If the header is invalid, but the requirements are still met,
     *   will throw a uhd::value_error.
     *
     * \param packet_buff memory to read the packed vrt header
     * \param if_packet_info the if packet info (read/write)
     */
    UHD_API void if_hdr_unpack_be(
        const boost::uint32_t *packet_buff,
        if_packet_info_t &if_packet_info
    );

    /*!
     * Pack a vrt header from metadata (little endian format).
     *
     * See \ref vrt_pack_contract.
     *
     * \param packet_buff memory to write the packed vrt header
     * \param if_packet_info the if packet info (read/write)
     */
    UHD_API void if_hdr_pack_le(
        boost::uint32_t *packet_buff,
        if_packet_info_t &if_packet_info
    );

    /*!
     * Unpack a vrt header to metadata (little endian format).
     *
     * See \ref vrt_unpack_contract.
     *
     * \param packet_buff memory to read the packed vrt header
     * \param if_packet_info the if packet info (read/write)
     */
    UHD_API void if_hdr_unpack_le(
        const boost::uint32_t *packet_buff,
        if_packet_info_t &if_packet_info
    );

    UHD_INLINE if_packet_info_t::if_packet_info_t(void):
        link_type(LINK_TYPE_NONE),
        packet_type(PACKET_TYPE_DATA),
        num_payload_words32(0),
        num_payload_bytes(0),
        num_header_words32(0),
        num_packet_words32(0),
        packet_count(0),
        sob(false), eob(false),
        error(false),
        has_sid(false), sid(0),
        has_cid(false), cid(0),
        has_tsi(false), tsi(0),
        has_tsf(false), tsf(0),
        has_tlr(false), tlr(0)
    {}

} //namespace vrt

}} //namespace

#endif /* INCLUDED_UHD_TRANSPORT_VRT_IF_PACKET_HPP */
