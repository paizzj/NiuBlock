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
#ifndef LIBBITCOIN_CHAIN_SCRIPT_HPP
#define LIBBITCOIN_CHAIN_SCRIPT_HPP

#include <cstddef>
#include <cstdint>
#include <istream>
#include <memory>
#include <string>
#include <bitcoin/bitcoin/define.hpp>
#include <bitcoin/bitcoin/error.hpp>
#include <bitcoin/bitcoin/math/elliptic_curve.hpp>
#include <bitcoin/bitcoin/machine/operation.hpp>
#include <bitcoin/bitcoin/machine/rule_fork.hpp>
#include <bitcoin/bitcoin/machine/script_pattern.hpp>
#include <bitcoin/bitcoin/utility/data.hpp>
#include <bitcoin/bitcoin/utility/reader.hpp>
#include <bitcoin/bitcoin/utility/thread.hpp>
#include <bitcoin/bitcoin/utility/writer.hpp>

namespace libbitcoin {
namespace chain {

class transaction;

class BC_API script
{
public:
    typedef machine::operation operation;

    // Constructors.
    //-------------------------------------------------------------------------

    script();

    script(script&& other);
    script(const script& other);

    script(operation::list&& ops);
    script(const operation::list& ops);

    script(data_chunk&& encoded, bool prefix);
    script(const data_chunk& encoded, bool prefix);

    // Operators.
    //-------------------------------------------------------------------------

    /// This class is move assignable and copy assignable.
    script& operator=(script&& other);
    script& operator=(const script& other);

    bool operator==(const script& other) const;
    bool operator!=(const script& other) const;

    // Deserialization.
    //-------------------------------------------------------------------------

    static script factory(const data_chunk& encoded, bool prefix);
    static script factory(std::istream& stream, bool prefix);
    static script factory(reader& source, bool prefix);

    /// Deserialization invalidates the iterator.
    bool from_data(const data_chunk& encoded, bool prefix);
    bool from_data(std::istream& stream, bool prefix);
    bool from_data(reader& source, bool prefix);

    /// Deserialization invalidates the iterator.
    void from_operations(operation::list&& ops);
    void from_operations(const operation::list& ops);
    bool from_string(const std::string& mnemonic);

    /// A script object is valid if the byte count matches the prefix.
    bool is_valid() const;

    /// Script operations is valid if all push ops have the predicated size.
    bool is_valid_operations() const;

    // Serialization.
    //-------------------------------------------------------------------------

    data_chunk to_data(bool prefix) const;
    void to_data(std::ostream& stream, bool prefix) const;
    void to_data(writer& sink, bool prefix) const;

    std::string to_string(uint32_t active_forks) const;

    // Iteration.
    //-------------------------------------------------------------------------

    void clear();
    bool empty() const;
    size_t size() const;
    const operation& front() const;
    const operation& back() const;
    operation::iterator begin() const;
    operation::iterator end() const;
    const operation& operator[](size_t index) const;

    // Properties (size, accessors, cache).
    //-------------------------------------------------------------------------

    size_t satoshi_content_size() const;
    size_t serialized_size(bool prefix) const;
    const operation::list& operations() const;

    // Signing.
    //-------------------------------------------------------------------------

    static hash_digest generate_signature_hash(const transaction& tx,
        uint32_t input_index, const script& script_code, uint8_t sighash_type);

    static bool check_signature(const ec_signature& signature,
        uint8_t sighash_type, const data_chunk& public_key,
        const script& script_code, const transaction& tx,
        uint32_t input_index);

    static bool create_endorsement(endorsement& out, const ec_secret& secret,
        const script& prevout_script, const transaction& tx,
        uint32_t input_index, uint8_t sighash_type);

    // Utilities (static).
    //-------------------------------------------------------------------------

    /// Determine if the fork is enabled in the active forks set.
    static bool is_enabled(uint32_t active_forks, machine::rule_fork fork)
    {
        return (fork & active_forks) != 0;
    }

    /// Consensus patterns.
    static bool is_push_only(const operation::list& ops);
    static bool is_relaxed_push(const operation::list& ops);
    static bool is_coinbase_pattern(const operation::list& ops, size_t height);

    /// Common output patterns (psh is also consensus).
    static bool is_null_data_pattern(const operation::list& ops);
    static bool is_pay_multisig_pattern(const operation::list& ops);
    static bool is_pay_public_key_pattern(const operation::list& ops);
    static bool is_pay_key_hash_pattern(const operation::list& ops);
    static bool is_pay_script_hash_pattern(const operation::list& ops);

    /// Common input patterns.
    static bool is_sign_multisig_pattern(const operation::list& ops);
    static bool is_sign_public_key_pattern(const operation::list& ops);
    static bool is_sign_key_hash_pattern(const operation::list& ops);
    static bool is_sign_script_hash_pattern(const operation::list& ops);

    /// Stack factories.
    static operation::list to_null_data_pattern(data_slice data);
    static operation::list to_pay_public_key_pattern(data_slice point);
    static operation::list to_pay_key_hash_pattern(const short_hash& hash);
    static operation::list to_pay_script_hash_pattern(const short_hash& hash);
    static operation::list to_pay_multisig_pattern(uint8_t signatures,
        const point_list& points);
    static operation::list to_pay_multisig_pattern(uint8_t signatures,
        const data_stack& points);

    // Utilities (non-static).
    //-------------------------------------------------------------------------

    // Common pattern detection.
    machine::script_pattern pattern() const;
    machine::script_pattern input_pattern() const;
    machine::script_pattern output_pattern() const;

    // Consensus computations.
    size_t sigops(bool embedded) const;
    size_t embedded_sigops(const script& prevout_script) const;
    void find_and_delete(const data_stack& endorsements);
    bool is_unspendable() const;

    // Validation.
    //-------------------------------------------------------------------------

    static code verify(const transaction& tx, uint32_t input, uint32_t forks);

    // TOD: move back to private.
    static code verify(const transaction& tx, uint32_t input_index,
        uint32_t forks, const script& input_script,
        const script& prevout_script);

protected:
    // So that input and output may call reset from their own.
    friend class input;
    friend class output;

    void reset();
    bool is_pay_to_script_hash(uint32_t forks) const;
    void find_and_delete_(const data_chunk& endorsement);

private:
    static size_t serialized_size(const operation::list& ops);
    static data_chunk operations_to_data(const operation::list& ops);

    operation::list& operations_move();
    const operation::list& operations_copy() const;

    // These are protected by mutex.
    mutable operation::list operations_;
    mutable bool cached_;
    mutable upgrade_mutex mutex_;

    data_chunk bytes_;
    bool valid_;
};

} // namespace chain
} // namespace libbitcoin

#endif
