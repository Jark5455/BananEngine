//
// Created by yashr on 4/19/24.
//

#include <coz.h>

#pragma once

#define EMPTY(n) (0 > (int)(index_[n].next))
#define EQHASH(n, key_hash) (((size_t)(key_hash) & ~mask_) == (index_[n].slot & ~mask_))

#define MAP_NEW(key, val, bucket, key_hash)                                             \
    new(pairs_ + num_filled_) value_type(key, val);                                     \
    etail_ = bucket;                                                                    \
    index_[bucket] = {bucket, num_filled_++ | ((size_t)(key_hash) & ~mask_)}

#define SET_NEW(val, bucket, hash)                                                      \
    new(pairs_ + num_filled_) value_type(val);                                          \
    etail_ = bucket;                                                                    \
    index_[bucket] = {bucket, num_filled_++ | ((size_t)(hash) & ~mask_)}


namespace BananSTLExt {
    template<class KeyT, class ValueT, class HashT, class EqT>
    void BananHashTable<KeyT, ValueT, HashT, EqT>::init(size_t bucket, float mlf) {
        pairs_ = nullptr;
        index_ = nullptr;
        mask_ = 0;
        num_buckets_ = 0;
        num_filled_ = 0;
        mlf_ = (uint32_t)((1 << 27) / DEFAULT_LOAD_FACTOR);
        max_load_factor(mlf);
        rehash(bucket);
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    BananHashTable<KeyT, ValueT, HashT, EqT>::BananHashTable(size_t bucket, float mlf) {
        init(bucket, mlf);
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    BananHashTable<KeyT, ValueT, HashT, EqT>::BananHashTable(const BananHashTable &rhs) {
        if (rhs.load_factor() > MIN_LOAD_FACTOR) {
            pairs_ = alloc_bucket((size_t)(rhs._num_buckets * rhs.max_load_factor()) + 4);
            index_ = alloc_index(rhs._num_buckets);
            clone(rhs);
        } else {
            init(rhs._num_filled + 2, MIN_LOAD_FACTOR);
            for (auto it = rhs.begin(); it != rhs.end(); ++it)
                insert_unique(it->first, it->second);
        }
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    BananHashTable<KeyT, ValueT, HashT, EqT>::BananHashTable(BananHashTable &&rhs) noexcept {
        init(0);
        *this = std::move(rhs);
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    BananHashTable<KeyT, ValueT, HashT, EqT> &BananHashTable<KeyT, ValueT, HashT, EqT>::operator=(const BananHashTable &rhs) {
        if (this == &rhs)
            return *this;

        if (rhs.load_factor() < MIN_LOAD_FACTOR) {
            clear();
            free(pairs_);
            pairs_ = nullptr;
            rehash(rhs._num_filled + 2);
            for (auto it = rhs.begin(); it != rhs.end(); ++it)
                insert_unique(it->first, it->second);
            return *this;
        }

        clearkv();

        if (num_buckets_ != rhs._num_buckets) {
            free(pairs_); free(index_);
            index_ = alloc_index(rhs._num_buckets);
            pairs_ = alloc_bucket((size_t)(rhs._num_buckets * rhs.max_load_factor()) + 4);
        }

        clone(rhs);
        return *this;
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    BananHashTable<KeyT, ValueT, HashT, EqT> &BananHashTable<KeyT, ValueT, HashT, EqT>::operator=(BananHashTable &&rhs) noexcept {
        if (this != &rhs) {
            swap(rhs);
            rhs.clear();
        }
        return *this;
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    template<typename Con>
    bool BananHashTable<KeyT, ValueT, HashT, EqT>::operator==(const Con &rhs) const {
        if (size() != rhs.size())
            return false;

        for (auto it = begin(), last = end(); it != last; ++it) {
            auto oi = rhs.find(it->first);
            if (oi == rhs.end() || it->second != oi->second)
                return false;
        }
        return true;
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    BananHashTable<KeyT, ValueT, HashT, EqT>::~BananHashTable() {
        clearkv();
        free(pairs_);
        free(index_);
        index_ = nullptr;
        pairs_ = nullptr;
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    void BananHashTable<KeyT, ValueT, HashT, EqT>::clone(const BananHashTable &rhs) {
        hasher_      = rhs.hasher_;
        eq_          = rhs.eq_;
        num_buckets_ = rhs.num_buckets_;
        num_filled_  = rhs.num_filled_;
        mlf_         = rhs.mlf_;
        last_        = rhs.last_;
        mask_        = rhs.mask_;
        etail_       = rhs.etail_;

        auto opairs  = rhs._pairs;
        memcpy((char*)index_, (char*)rhs._index, (num_buckets_ + EAD) * sizeof(Index));

        if (is_copy_trivially()) {
            memcpy((char*)pairs_, (char*)opairs, num_filled_ * sizeof(value_type));
        } else {
            for (size_t slot = 0; slot < num_filled_; slot++)
                new(pairs_ + slot) value_type(opairs[slot]);
        }
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    void BananHashTable<KeyT, ValueT, HashT, EqT>::swap(BananHashTable &rhs) {
        std::swap(hasher_, rhs._hasher);
        std::swap(pairs_, rhs._pairs);
        std::swap(index_, rhs._index);
        std::swap(num_buckets_, rhs._num_buckets);
        std::swap(num_filled_, rhs._num_filled);
        std::swap(mask_, rhs._mask);
        std::swap(mlf_, rhs._mlf);
        std::swap(last_, rhs._last);
        std::swap(etail_, rhs._etail);
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    template<typename K>
    std::pair<typename BananHashTable<KeyT, ValueT, HashT, EqT>::iterator, typename BananHashTable<KeyT, ValueT, HashT, EqT>::iterator> BananHashTable<KeyT, ValueT, HashT, EqT>::equal_range(const K &key) {
        const auto found = find(key);
        if (found.second == num_filled_) return { found, found };
        else return { found, std::next(found) };
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    void BananHashTable<KeyT, ValueT, HashT, EqT>::merge(BananHashTable &rhs) {
        if (empty()) {
            *this = std::move(rhs);
            return;
        }

        for (auto rit = rhs.begin(); rit != rhs.end(); ) {
            auto fit = find(rit->first);
            if (fit == end()) {
                insert_unique(rit->first, std::move(rit->second));
                rit = rhs.erase(rit);
            } else {
                ++rit;
            }
        }
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    template<typename V>
    bool BananHashTable<KeyT, ValueT, HashT, EqT>::try_get(const KeyT &key, V &val) const noexcept requires (!std::is_void_v<ValueT>) {
        const auto slot = find_filled_slot(key);
        const auto found = slot != num_filled_;
        if (found) {
            val = pairs_[slot].second;
        }
        return found;
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    template<typename V>
    V *BananHashTable<KeyT, ValueT, HashT, EqT>::try_get(const KeyT &key) const noexcept requires (!std::is_void_v<ValueT>) {
        const auto slot = find_filled_slot(key);
        return slot != num_filled_ ? &pairs_[slot].second : nullptr;
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    template<typename V>
    bool BananHashTable<KeyT, ValueT, HashT, EqT>::try_set(const KeyT &key, const V &val) noexcept requires (!std::is_void_v<ValueT>) {
        const auto slot = find_filled_slot(key);
        if (slot == num_filled_)
            return false;

        pairs_[slot].second = val;
        return true;
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    template<typename V>
    bool BananHashTable<KeyT, ValueT, HashT, EqT>::try_set(const KeyT &key, V &&val) noexcept requires (!std::is_void_v<ValueT>) {
        const auto slot = find_filled_slot(key);
        if (slot == num_filled_)
            return false;

        pairs_[slot].second = std::move(val);
        return true;
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    template<typename V>
    V BananHashTable<KeyT, ValueT, HashT, EqT>::get_or_return_default(const KeyT &key) const noexcept requires (!std::is_void_v<ValueT>) {
        const auto slot = find_filled_slot(key);
        return slot == num_filled_ ? ValueT() : pairs_[slot].second;
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    std::pair<typename BananHashTable<KeyT, ValueT, HashT, EqT>::iterator, bool>
    BananHashTable<KeyT, ValueT, HashT, EqT>::do_insert(const BananHashTable::value_type &value) noexcept {
        COZ_BEGIN("DO_INSERT")

        const auto key_hash = hash_key(get_key(value));
        const auto bucket = find_or_allocate(get_key(value), key_hash);
        const auto bempty = EMPTY(bucket);

        if (bempty) {
            if constexpr (!std::is_void_v<ValueT>) {
                MAP_NEW(value.first, value.second, bucket, key_hash);
            } else {
                SET_NEW(value, bucket, key_hash);
            }
        }

        const auto slot = index_[bucket].slot & mask_;

        COZ_END("DO_INSERT")
        return { {this, slot}, bempty };
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    std::pair<typename BananHashTable<KeyT, ValueT, HashT, EqT>::iterator, bool>
    BananHashTable<KeyT, ValueT, HashT, EqT>::do_insert(BananHashTable::value_type &&value) noexcept {
        COZ_BEGIN("DO_INSERT2")

        uint64_t key_hash;
        size_t bucket;

        if constexpr (!std::is_void_v<ValueT>) {
            key_hash = hash_key(value.first);
            bucket = find_or_allocate(value.first, key_hash);
        } else {
            key_hash = hash_key(value);
            bucket = find_or_allocate(value, key_hash);
        }

        const auto bempty = EMPTY(bucket);

        if (bempty) {
            if constexpr (!std::is_void_v<ValueT>) {
                MAP_NEW(std::move(value.first), std::move(value.second), bucket, key_hash);
            } else {
                SET_NEW(std::move(value), bucket, key_hash);
            }
        }

        const auto slot = index_[bucket].slot & mask_;

        COZ_END("DO_INSERT2")
        return { {this, slot}, bempty };
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    template<typename K, typename V>
    std::pair<typename BananHashTable<KeyT, ValueT, HashT, EqT>::iterator, bool>
    BananHashTable<KeyT, ValueT, HashT, EqT>::do_insert(K &&key, V &&val) noexcept requires (!std::is_void_v<ValueT>) {
        COZ_BEGIN("DO_INSERT3")

        const auto key_hash = hash_key(key);
        const auto bucket = find_or_allocate(key, key_hash);
        const auto bempty = EMPTY(bucket);

        if (bempty) {
            MAP_NEW(std::forward<K>(key), std::forward<V>(val), bucket, key_hash);
        }

        const auto slot = index_[bucket].slot & mask_;

        COZ_END("DO_INSERT3")
        return { {this, slot}, bempty };
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    template<typename K, typename V>
    std::pair<typename BananHashTable<KeyT, ValueT, HashT, EqT>::iterator, bool>
    BananHashTable<KeyT, ValueT, HashT, EqT>::do_assign(K &&key, V &&val) noexcept requires (!std::is_void_v<ValueT>) {
        check_expand_need();
        const auto key_hash = hash_key(key);
        const auto bucket = find_or_allocate(key, key_hash);
        const auto bempty = EMPTY(bucket);
        if (bempty) {
            MAP_NEW(std::forward<K>(key), std::forward<V>(val), bucket, key_hash);
        } else {
            pairs_[index_[bucket].slot & mask_].second = std::forward<V>(val);
        }

        const auto slot = index_[bucket].slot & mask_;
        return { {this, slot}, bempty };
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    std::pair<typename BananHashTable<KeyT, ValueT, HashT, EqT>::iterator, bool>
    BananHashTable<KeyT, ValueT, HashT, EqT>::insert(const BananHashTable::value_type &p) {
        check_expand_need();
        return do_insert(p);
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    std::pair<typename BananHashTable<KeyT, ValueT, HashT, EqT>::iterator, bool>
    BananHashTable<KeyT, ValueT, HashT, EqT>::insert(BananHashTable::value_type &&p) {
        check_expand_need();
        return do_insert(std::move(p));
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    void BananHashTable<KeyT, ValueT, HashT, EqT>::insert(std::initializer_list<value_type> ilist) {
        reserve(ilist.size() + num_filled_, false);
        for (auto it = ilist.begin(); it != ilist.end(); ++it)
            do_insert(*it);
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    template<typename Iter>
    void BananHashTable<KeyT, ValueT, HashT, EqT>::insert(Iter first, Iter last) {
        reserve(std::distance(first, last) + num_filled_, false);
        for (; first != last; ++first)
            do_insert(first->first, first->second);
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    template<typename K, typename V>
    size_t BananHashTable<KeyT, ValueT, HashT, EqT>::insert_unique(K &&key, V &&val) requires (!std::is_void_v<ValueT>) {
        check_expand_need();
        const auto key_hash = hash_key(key);
        auto bucket = find_unique_bucket(key_hash);
        NEW(std::forward<K>(key), std::forward<V>(val), bucket, key_hash);
        return bucket;
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    size_t BananHashTable<KeyT, ValueT, HashT, EqT>::insert_unique(BananHashTable::value_type &&value) {
        return insert_unique(std::move(value.first), std::move(value.second));
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    size_t BananHashTable<KeyT, ValueT, HashT, EqT>::insert_unique(const BananHashTable::value_type &value) {
        return insert_unique(value.first, value.second);
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    template<class... Args>
    std::pair<typename BananHashTable<KeyT, ValueT, HashT, EqT>::iterator, bool> BananHashTable<KeyT, ValueT, HashT, EqT>::emplace(Args &&... args) noexcept {
        check_expand_need();
        return do_insert(std::forward<Args>(args)...);
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    template<class... Args>
    BananHashTable<KeyT, ValueT, HashT, EqT>::iterator
    BananHashTable<KeyT, ValueT, HashT, EqT>::emplace_hint(BananHashTable::const_iterator hint, Args &&... args) {
        (void)hint;
        check_expand_need();
        return do_insert(std::forward<Args>(args)...).first;
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    template<class... Args>
    std::pair<typename BananHashTable<KeyT, ValueT, HashT, EqT>::iterator, bool>
    BananHashTable<KeyT, ValueT, HashT, EqT>::try_emplace(const KeyT &k, Args &&... args) {
        check_expand_need();
        return do_insert(k, std::forward<Args>(args)...);
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    template<class... Args>
    std::pair<typename BananHashTable<KeyT, ValueT, HashT, EqT>::iterator, bool>
    BananHashTable<KeyT, ValueT, HashT, EqT>::try_emplace(KeyT &&k, Args &&... args) {
        check_expand_need();
        return do_insert(std::move(k), std::forward<Args>(args)...);
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    template<class... Args>
    size_t BananHashTable<KeyT, ValueT, HashT, EqT>::emplace_unique(Args &&... args) {
        return insert_unique(std::forward<Args>(args)...);
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    template<typename V>
    V BananHashTable<KeyT, ValueT, HashT, EqT>::set_get(const KeyT &key, const V &val) requires (!std::is_void_v<ValueT>) {
        check_expand_need();
        const auto key_hash = hash_key(key);
        const auto bucket = find_or_allocate(key, key_hash);
        if (EMPTY(bucket)) {
            MAP_NEW(key, val, bucket, key_hash);
            return ValueT();
        } else {
            const auto slot = index_[bucket].slot & mask_;
            ValueT old_value(val);
            std::swap(pairs_[slot].second, old_value);
            return old_value;
        }
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    template<typename V>
    V &BananHashTable<KeyT, ValueT, HashT, EqT>::operator[](const KeyT &key) noexcept requires (!std::is_void_v<ValueT>) {
        check_expand_need();
        const auto key_hash = hash_key(key);
        const auto bucket = find_or_allocate(key, key_hash);
        if (EMPTY(bucket)) {
            /* Check if inserting a value rather than overwriting an old entry */
            MAP_NEW(key, std::move(ValueT()), bucket, key_hash);
        }

        const auto slot = index_[bucket].slot & mask_;
        return pairs_[slot].second;
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    template<typename V>
    V &BananHashTable<KeyT, ValueT, HashT, EqT>::operator[](KeyT &&key) noexcept requires (!std::is_void_v<ValueT>) {
        check_expand_need();
        const auto key_hash = hash_key(key);
        const auto bucket = find_or_allocate(key, key_hash);
        if (EMPTY(bucket)) {
            MAP_NEW(std::move(key), std::move(ValueT()), bucket, key_hash);
        }

        const auto slot = index_[bucket].slot & mask_;
        return pairs_[slot].second;
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    size_t BananHashTable<KeyT, ValueT, HashT, EqT>::erase(const KeyT &key) noexcept {
        const auto key_hash = hash_key(key);
        const auto sbucket = find_filled_bucket(key, key_hash);
        if (sbucket == INACTIVE)
            return 0;

        const auto main_bucket = key_hash & mask_;
        erase_slot(sbucket, (size_t)main_bucket);
        return 1;
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    BananHashTable<KeyT, ValueT, HashT, EqT>::iterator
    BananHashTable<KeyT, ValueT, HashT, EqT>::erase(const BananHashTable::const_iterator &cit) noexcept {
        const auto slot = (size_t)(cit.kv_ - pairs_);
        size_t main_bucket;
        const auto sbucket = find_slot_bucket(slot, main_bucket); //TODO
        erase_slot(sbucket, main_bucket);
        return {this, slot};
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    BananHashTable<KeyT, ValueT, HashT, EqT>::iterator
    BananHashTable<KeyT, ValueT, HashT, EqT>::erase(BananHashTable::const_iterator first, BananHashTable::const_iterator last) noexcept {
        auto esize = long(last.kv_ - first.kv_);
        auto tsize = long((pairs_ + num_filled_) - last.kv_); //last to tail size
        auto next = first;
        while (tsize -- > 0) {
            if (esize-- <= 0)
                break;
            next = ++erase(next);
        }

        //fast erase from last
        next = this->last();
        while (esize -- > 0)
            next = --erase(next);

        return {this, size_t(next.kv_ - pairs_)};
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    template<typename Pred>
    size_t BananHashTable<KeyT, ValueT, HashT, EqT>::erase_if(Pred pred) {
        auto old_size = size();
        for (auto it = begin(); it != end();) {
            if (pred(*it))
                it = erase(it);
            else
                ++it;
        }
        return old_size - size();
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    void BananHashTable<KeyT, ValueT, HashT, EqT>::clearkv() {
        if (is_triviall_destructable()) {
            while (num_filled_ --)
                pairs_[num_filled_].~value_type();
        }
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    void BananHashTable<KeyT, ValueT, HashT, EqT>::clear() noexcept {
        clearkv();

        if (num_filled_ > 0)
            memset((char*)index_, INACTIVE, sizeof(index_[0]) * num_buckets_);

        last_ = num_filled_ = 0;
        etail_ = INACTIVE;
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    void BananHashTable<KeyT, ValueT, HashT, EqT>::shrink_to_fit(const float min_factor) {
        if (load_factor() < min_factor && bucket_count() > 10) //safe guard
            rehash(num_filled_ + 1);
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    bool BananHashTable<KeyT, ValueT, HashT, EqT>::reserve(uint64_t num_elems, bool force) {
        (void)force;
        const auto required_buckets = num_elems * mlf_ >> 27;
        if (required_buckets < mask_) [[likely]] // && !force
            return false;

        rehash(required_buckets + 2);
        return true;
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    BananHashTable<KeyT, ValueT, HashT, EqT>::value_type* BananHashTable<KeyT, ValueT, HashT, EqT>::alloc_bucket(size_t num_buckets)
    {
        auto new_pairs = malloc((uint64_t)num_buckets * sizeof(value_type));
        return (value_type *)(new_pairs);
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    BananHashTable<KeyT, ValueT, HashT, EqT>::Index* BananHashTable<KeyT, ValueT, HashT, EqT>::alloc_index(size_t num_buckets)
    {
        auto new_index = (char*)malloc((uint64_t)(EAD + num_buckets) * sizeof(Index));
        return (Index *)(new_index);
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    bool BananHashTable<KeyT, ValueT, HashT, EqT>::reserve(size_t required_buckets) noexcept {
        if (num_filled_ != required_buckets)
            return reserve(required_buckets, true);

        last_ = 0;

        memset((char*)index_, INACTIVE, sizeof(index_[0]) * num_buckets_);
        for (size_t slot = 0; slot < num_filled_; slot++) {
            const auto& key = pairs_[slot].first;
            const auto key_hash = hash_key(key);
            const auto bucket = size_t(key_hash & mask_);
            auto& next_bucket = index_[bucket].next;
            if ((int)next_bucket < 0)
                index_[bucket] = {1, slot | ((size_t)(key_hash) & ~mask_)};
            else {
                index_[bucket].slot |= (size_t)(key_hash) & ~mask_;
                next_bucket ++;
            }
        }
        return true;
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    void BananHashTable<KeyT, ValueT, HashT, EqT>::rebuild(size_t num_buckets) noexcept {
        COZ_BEGIN("REBUILD")

        free(index_);
        auto new_pairs = (value_type*)alloc_bucket((size_t)(num_buckets * max_load_factor()) + 4);
        if (is_copy_trivially()) {
            if (pairs_)
                memcpy((char*)new_pairs, (char*)pairs_, num_filled_ * sizeof(value_type));
        } else {
            for (size_t slot = 0; slot < num_filled_; slot++) {
                new(new_pairs + slot) value_type(std::move(pairs_[slot]));
                if (is_triviall_destructable())
                    pairs_[slot].~value_type();
            }
        }
        free(pairs_);
        pairs_ = new_pairs;
        index_ = (Index*)alloc_index(num_buckets);

        memset((char*)index_, INACTIVE, sizeof(index_[0]) * num_buckets);
        memset((char*)(index_ + num_buckets), 0, sizeof(index_[0]) * EAD);

        COZ_END("REBUILD")
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    void BananHashTable<KeyT, ValueT, HashT, EqT>::rehash(uint64_t required_buckets) {
        COZ_BEGIN("REHASH")

        if (required_buckets < num_filled_)
            return;

        assert(required_buckets < max_size());
        auto num_buckets = num_filled_ > (1u << 16) ? (1u << 16) : 4u;
        while (num_buckets < required_buckets) { num_buckets *= 2; }

        last_ = 0;
        mask_ = num_buckets - 1;
        num_buckets_ = num_buckets;
        rebuild(num_buckets);

        etail_ = INACTIVE;
        for (size_t slot = 0; slot < num_filled_; ++slot) {
            const auto &key = get_key(pairs_[slot]);
            const auto key_hash = hash_key(key);
            const auto bucket = find_unique_bucket(key_hash);
            index_[bucket] = {bucket, slot | ((size_t) (key_hash) & ~mask_)};
        }

        COZ_END("REHASH")
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    bool BananHashTable<KeyT, ValueT, HashT, EqT>::check_expand_need() {
        return reserve(num_filled_, false);
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    void BananHashTable<KeyT, ValueT, HashT, EqT>::prefetch_heap_block(char *ctrl) {
#if __linux__
        __builtin_prefetch(static_cast<const void*>(ctrl));
#elif _WIN32
        _mm_prefetch((const char*)ctrl, _MM_HINT_T0);
#endif
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    size_t BananHashTable<KeyT, ValueT, HashT, EqT>::slot_to_bucket(const size_t slot) const noexcept {
        size_t main_bucket;
        return find_slot_bucket(slot, main_bucket);
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    void BananHashTable<KeyT, ValueT, HashT, EqT>::erase_slot(const size_t sbucket, const size_t main_bucket) noexcept {
        const auto slot = index_[sbucket].slot & mask_;
        const auto ebucket = erase_bucket(sbucket, main_bucket);
        const auto last_slot = --num_filled_;
        if (slot != last_slot) [[likely]] {
            const auto last_bucket = (etail_ == INACTIVE || ebucket == etail_) ? slot_to_bucket(last_slot) : etail_;

            pairs_[slot] = std::move(pairs_[last_slot]);
            index_[last_bucket].slot = slot | (index_[last_bucket].slot & ~mask_);
        }

        if (is_triviall_destructable())
            pairs_[last_slot].~value_type();

        etail_ = INACTIVE;
        index_[ebucket] = {INACTIVE, 0};
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    size_t BananHashTable<KeyT, ValueT, HashT, EqT>::erase_bucket(const size_t bucket, const size_t main_bucket) noexcept {
        const auto next_bucket = index_[bucket].next;
        if (bucket == main_bucket) {
            if (main_bucket != next_bucket) {
                const auto nbucket = index_[next_bucket].next;
                index_[main_bucket] = {
                        (nbucket == next_bucket) ? main_bucket : nbucket,
                        index_[next_bucket].slot
                };
            }
            return next_bucket;
        }

        const auto prev_bucket = find_prev_bucket(main_bucket, bucket);
        index_[prev_bucket].next = (bucket == next_bucket) ? prev_bucket : next_bucket;
        return bucket;
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    size_t BananHashTable<KeyT, ValueT, HashT, EqT>::find_slot_bucket(const size_t slot, size_t &main_bucket) const {
        const auto key_hash = hash_key(pairs_[slot].first);
        const auto bucket = main_bucket = size_t(key_hash & mask_);
        if (slot == (index_[bucket].slot & mask_))
            return bucket;

        auto next_bucket = index_[bucket].next;
        while (true) {
            if (slot == (index_[next_bucket].slot & mask_)) [[likely]]
                return next_bucket;
            next_bucket = index_[next_bucket].next;
        }

        return INACTIVE;
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    size_t BananHashTable<KeyT, ValueT, HashT, EqT>::find_filled_bucket(const KeyT &key, uint64_t key_hash) const noexcept {
        const auto bucket = size_t (key_hash & mask_);
        auto next_bucket  = index_[bucket].next;
        if ((int)next_bucket < 0) [[unlikely]]
            return INACTIVE;

        const auto slot = index_[bucket].slot & mask_;
        prefetch_heap_block((char*)&pairs_[slot]);
        if (EQHASH(bucket, key_hash)) {
            if (eq_(key, pairs_[slot].first)) [[likely]]
                return bucket;
        }
        if (next_bucket == bucket)
            return INACTIVE;

        while (true) {
            if (EQHASH(next_bucket, key_hash)) {
                const auto next_slot = index_[next_bucket].slot & mask_;
                if (eq_(key, pairs_[next_slot].first)) [[likely]]
                    return next_bucket;
            }

            const auto nbucket = index_[next_bucket].next;
            if (nbucket == next_bucket)
                return INACTIVE;
            next_bucket = nbucket;
        }

        return INACTIVE;
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    template<typename K>
    size_t BananHashTable<KeyT, ValueT, HashT, EqT>::find_filled_slot(const K &key) const noexcept {
        const auto key_hash = hash_key(key);
        const auto bucket = size_t(key_hash & mask_);
        auto next_bucket = index_[bucket].next;
        if ((int)next_bucket < 0)
            return num_filled_;

        const auto slot = index_[bucket].slot & mask_;
        prefetch_heap_block((char*)&pairs_[slot]);
        if (EQHASH(bucket, key_hash)) {
            if (eq_(key, get_key(pairs_[slot]))) [[likely]]
                return slot;
        }
        if (next_bucket == bucket)
            return num_filled_;

        while (true) {
            if (EQHASH(next_bucket, key_hash)) {
                const auto next_slot = index_[next_bucket].slot & mask_;
                if (eq_(key, get_key(pairs_[next_slot]))) [[likely]]
                    return slot;
            }

            const auto nbucket = index_[next_bucket].next;
            if (nbucket == next_bucket)
                return num_filled_;
            next_bucket = nbucket;
        }

        return num_filled_;
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    size_t BananHashTable<KeyT, ValueT, HashT, EqT>::kickout_bucket(const size_t kmain, const size_t bucket) noexcept {
        const auto next_bucket = index_[bucket].next;
        const auto new_bucket  = find_empty_bucket(next_bucket, 2);
        const auto prev_bucket = find_prev_bucket(kmain, bucket);

        const auto last = next_bucket == bucket ? new_bucket : next_bucket;
        index_[new_bucket] = {last, index_[bucket].slot};

        index_[prev_bucket].next = new_bucket;
        index_[bucket].next = INACTIVE;

        return bucket;
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    template<typename K>
    size_t BananHashTable<KeyT, ValueT, HashT, EqT>::find_or_allocate(const K &key, uint64_t key_hash) noexcept {
        const auto bucket = size_t(key_hash & mask_);
        auto next_bucket = index_[bucket].next;
        prefetch_heap_block((char*)&pairs_[bucket]);
        if ((int)next_bucket < 0) {
            return bucket;
        }

        const auto slot = index_[bucket].slot & mask_;
        if (EQHASH(bucket, key_hash))
            if (eq_(key, get_key(pairs_[slot]))) [[likely]]
                return bucket;

        //check current bucket_key is in main bucket or not
        const auto kmain = hash_bucket(get_key(pairs_[slot]));
        if (kmain != bucket)
            return kickout_bucket(kmain, bucket);
        else if (next_bucket == bucket)
            return index_[next_bucket].next = find_empty_bucket(next_bucket, 1);

        uint32_t csize = 1;
        //find next linked bucket and check key
        while (true) {
            const auto eslot = index_[next_bucket].slot & mask_;
            if (EQHASH(next_bucket, key_hash)) {
                if (eq_(key, get_key(pairs_[eslot]))) [[likely]]
                    return next_bucket;
            }

            csize += 1;
            const auto nbucket = index_[next_bucket].next;
            if (nbucket == next_bucket)
                break;
            next_bucket = nbucket;
        }

        //find a empty and link it to tail
        const auto new_bucket = find_empty_bucket(next_bucket, csize);
        prefetch_heap_block((char*)&pairs_[new_bucket]);
        return index_[next_bucket].next = new_bucket;
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    size_t BananHashTable<KeyT, ValueT, HashT, EqT>::find_unique_bucket(uint64_t key_hash) noexcept {
        const auto bucket = size_t(key_hash & mask_);
        auto next_bucket = index_[bucket].next;
        if ((int)next_bucket < 0) {
            return bucket;
        }

        const auto kmain = hash_main(bucket);
        if (kmain != bucket) [[unlikely]]
            return kickout_bucket(kmain, bucket);
        else if (next_bucket != bucket) [[unlikely]]
            next_bucket = find_last_bucket(next_bucket);

        return index_[next_bucket].next = find_empty_bucket(next_bucket, 2);
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    size_t BananHashTable<KeyT, ValueT, HashT, EqT>::find_empty_bucket(const size_t bucket_from, uint32_t csize) noexcept {
        (void)csize;
        auto bucket = bucket_from;
        if (EMPTY(++bucket) || EMPTY(++bucket))
            return bucket;

        const size_t linear_probe_length = 2 * CACHE_LINE_SIZE / sizeof(Index); //16
        for (size_t offset = csize + 2, step = 4; offset <= linear_probe_length; ) {
            bucket = (bucket_from + offset) & mask_;
            if (EMPTY(bucket) || EMPTY(++bucket))
                return bucket;
            offset += step;
        }

        last_ &= mask_;
        if (EMPTY(++last_))// || EMH_EMPTY(++_last))
            return last_;

        for (;;) {
            last_ &= mask_;
            if (EMPTY(++last_))// || EMH_EMPTY(++_last))
                return last_;

            auto _medium = (num_buckets_ / 2 + last_) & mask_;
            if (EMPTY(_medium))// || EMH_EMPTY(++_medium))
                return _medium;
        }

        return 0;
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    size_t BananHashTable<KeyT, ValueT, HashT, EqT>::find_last_bucket(size_t main_bucket) const {
        auto next_bucket = index_[main_bucket].next;
        if (next_bucket == main_bucket)
            return main_bucket;

        while (true) {
            const auto nbucket = index_[next_bucket].next;
            if (nbucket == next_bucket)
                return next_bucket;
            next_bucket = nbucket;
        }
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    size_t BananHashTable<KeyT, ValueT, HashT, EqT>::find_prev_bucket(const size_t main_bucket, const size_t bucket) const {
        auto next_bucket = index_[main_bucket].next;
        if (next_bucket == bucket)
            return main_bucket;

        while (true) {
            const auto nbucket = index_[next_bucket].next;
            if (nbucket == bucket)
                return next_bucket;
            next_bucket = nbucket;
        }
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    KeyT BananHashTable<KeyT, ValueT, HashT, EqT>::get_key(BananHashTable::value_type v) noexcept {
        if constexpr (!std::is_void_v<ValueT>) {
            return v.first;
        } else {
            return v;
        }
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    size_t BananHashTable<KeyT, ValueT, HashT, EqT>::hash_bucket(const KeyT &key) const noexcept {
        return (size_t)hash_key(key) & mask_;
    }

    template<class KeyT, class ValueT, class HashT, class EqT>
    size_t BananHashTable<KeyT, ValueT, HashT, EqT>::hash_main(const size_t bucket) const noexcept {
        const auto slot = index_[bucket].slot & mask_;
        return (size_t)hash_key(get_key(pairs_[slot])) & mask_;
    }
}