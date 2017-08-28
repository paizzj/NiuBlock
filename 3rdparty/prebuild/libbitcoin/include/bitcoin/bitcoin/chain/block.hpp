/**
 * Copyright (c) 2011-2017 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LIBBITCOIN_CHAIN_BLOCK_HPP
#define LIBBITCOIN_CHAIN_BLOCK_HPP

#include <cstddef>
#include <cstdint>
#include <istream>
#include <memory>
#include <string>
#include <vector>
#include <boost/optional.hpp>
#include <bitcoin/bitcoin/chain/chain_state.hpp>
#include <bitcoin/bitcoin/chain/header.hpp>
#include <bitcoin/bitcoin/chain/transaction.hpp>
#include <bitcoin/bitcoin/define.hpp>
#include <bitcoin/bitcoin/error.hpp>
#include <bitcoin/bitcoin/math/hash.hpp>
#include <bitcoin/bitcoin/utility/asio.hpp>
#include <bitcoin/bitcoin/utility/data.hpp>
#include <bitcoin/bitcoin/utility/reader.hpp>
#include <bitcoin/bitcoin/utility/thread.hpp>
#include <bitcoin/bitcoin/utility/writer.hpp>

namespace libbitcoin {
namespace chain {

class BC_API block
{
public:
    typedef std::vector<block> list;
    typedef std::vector<size_t> indexes;

    // THIS IS FOR LIBRARY USE ONLY, DO NOT CREATE A DEPENDENCY ON IT.
    struct validation
    {
        uint64_t originator = 0;
        code error = error::not_found;
        chain_state::ptr state = nullptr;

        // Similate organization and instead just validate the block.
        bool simulate = false;

        asio::time_point start_deserialize;
        asio::time_point end_deserialize;
        asio::time_point start_check;
        asio::time_point start_populate;
        asio::time_point start_accept;
        asio::time_point start_connect;
        asio::time_point start_notify;
        asio::time_point start_pop;
        asio::time_point start_push;
        asio::time_point end_push;
        float cache_efficiency;
    };

    // Constructors.
    //-------------------------------------------------------------------------

    block();

    block(block&& other);
    block(const block& other);

    block(chain::header&& header, transaction::list&& transactions);
    block(const chain::header& header, const transaction::list& transactions);

    // Operators.
    //-------------------------------------------------------------------------

    /// This class is move assignable but NOT copy assignable (performance).
    block& operator=(block&& other);
    block& operator=(const block& other) = delete;

    bool operator==(const block& other) const;
    bool operator!=(const block& other) const;

    // Deserialization.
    //-------------------------------------------------------------------------

    static block factory(const data_chunk& data);
    static block factory(std::istream& stream);
    static block factory(reader& source);

    bool from_data(const data_chunk& data);
    bool from_data(std::istream& stream);
    bool from_data(reader& source);

    bool is_valid() const;

    // Serialization.
    //-------------------------------------------------------------------------

    data_chunk to_data() const;
    void to_data(std::ostream& stream) const;
    void to_data(writer& sink) const;
    hash_list to_hashes() const;

    // Properties (size, accessors, cache).
    //-------------------------------------------------------------------------

    size_t serialized_size() const;

    chain::header& header();
    const chain::header& header() const;
    void set_header(const chain::header& value);
    void set_header(chain::header&& value);

    const transaction::list& transactions() const;
    void set_transactions(const transaction::list& value);
    void set_transactions(transaction::list&& value);

    hash_digest hash() const;

    // Utilities.
    //-------------------------------------------------------------------------

    static block genesis_mainnet();
    static block genesis_testnet();
    static size_t locator_size(size_t top);
    static indexes locator_heights(size_t top);

    // Validation.
    //-------------------------------------------------------------------------

    static uint64_t subsidy(size_t height);
    static uint256_t proof(uint32_t bits);

    uint64_t fees() const;
    uint64_t claim() const;
    uint64_t reward(size_t height) const;
    uint256_t proof() const;
    hash_digest generate_merkle_root() const;
    size_t signature_operations() const;
    size_t signature_operations(bool bip16_active) const;
    size_t total_non_coinbase_inputs() const;
    size_t total_inputs() const;

    bool is_extra_coinbases() const;
    bool is_final(size_t height, uint32_t block_time) const;
    bool is_distinct_transaction_set() const;
    bool is_valid_coinbase_claim(size_t height) const;
    bool is_valid_coinbase_script(size_t height) const;
    bool is_internal_double_spend() const;
    bool is_valid_merkle_root() const;

    code check() const;
    code check_transactions() const;
    code accept(bool transactions=true, bool header=true) const;
    code accept(const chain_state& state, bool transactions=true,
        bool header=true) const;
    code accept_transactions(const chain_state& state) const;
    code connect() const;
    code connect(const chain_state& state) const;
    code connect_transactions(const chain_state& state) const;

    // THIS IS FOR LIBRARY USE ONLY, DO NOT CREATE A DEPENDENCY ON IT.
    mutable validation validation;

protected:
    void reset();

private:
    typedef boost::optional<size_t> optional_size;

    optional_size total_inputs_cache() const;
    optional_size non_coinbase_inputs_cache() const;

    chain::header header_;
    transaction::list transactions_;

    mutable optional_size total_inputs_;
    mutable optional_size non_coinbase_inputs_;
    mutable upgrade_mutex mutex_;
};

} // namespace chain
} // namespace libbitcoin

#endif
