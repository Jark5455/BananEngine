//
// Created by yashr on 4/19/24.
//

// Modified version of ktprime emhash

#pragma once

#include <utility>
#include <vector>
#include <span>
#include <cstdint>

#include "hash.h"
#include "cpuinfo.h"

namespace BananSTLExt {

    template <class KeyT, class ValueT, class HashT, class EqT> class BananHashTable {
        constexpr static float DEFAULT_LOAD_FACTOR = 0.80f;
        constexpr static float MIN_LOAD_FACTOR = 0.25f;

        const uint32_t CACHE_LINE_SIZE = CPUInfo::getCPUInfo().CACHE_LINE_SIZE;

        public:
            typedef BananHashTable<KeyT, ValueT, HashT, EqT> htype;
            typedef std::conditional_t<std::is_void_v<ValueT>, KeyT, std::pair<KeyT, ValueT>> value_type;

            constexpr static size_t INACTIVE = 0-1u;
            constexpr static size_t EAD = 2;

            struct Index
            {
                size_t next;
                size_t slot;
            };

        private:
            Index *index_;
            value_type *pairs_;
            HashT hasher_;
            EqT eq_;
            uint32_t mlf_;
            size_t mask_;
            size_t num_buckets_;
            size_t num_filled_;
            size_t last_;
            size_t etail_;

        public:
            class const_iterator;
            class iterator {
                public:
                    using iterator_category = std::bidirectional_iterator_tag;
                    using difference_type = std::ptrdiff_t;
                    using value_type      = typename htype::value_type;
                    using pointer         = value_type*;
                    using const_pointer   = const value_type*;
                    using reference       = value_type&;
                    using const_reference = const value_type&;

                    iterator() : kv_(nullptr) {}

                    iterator(const_iterator& cit) {
                        kv_ = cit.kv_;
                    }

                    iterator(htype* hash_map, size_t bucket) {
                        kv_ = hash_map->pairs_ + (int)bucket;
                    }

                    iterator& operator++()
                    {
                        kv_ ++;
                        return *this;
                    }

                    iterator operator++(int)
                    {
                        auto cur = *this; kv_ ++;
                        return cur;
                    }

                    iterator& operator--()
                    {
                        kv_ --;
                        return *this;
                    }

                    iterator operator--(int)
                    {
                        auto cur = *this; kv_ --;
                        return cur;
                    }

                    reference operator*() const { return *kv_; }
                    pointer operator->() const { return kv_; }

                    bool operator == (const iterator& rhs) const { return kv_ == rhs.kv_; }
                    bool operator != (const iterator& rhs) const { return kv_ != rhs.kv_; }
                    bool operator == (const const_iterator& rhs) const { return kv_ == rhs.kv_; }
                    bool operator != (const const_iterator& rhs) const { return kv_ != rhs.kv_; }

                public:
                    value_type* kv_;
            };

            class const_iterator {
                public:
                    using iterator_category = std::bidirectional_iterator_tag;
                    using value_type        = typename htype::value_type;
                    using difference_type   = std::ptrdiff_t;
                    using pointer           = value_type*;
                    using const_pointer     = const value_type*;
                    using reference         = value_type&;
                    using const_reference   = const value_type&;

                    const_iterator(const iterator& it) {
                        kv_ = it.kv_;
                    }

                    const_iterator (const htype* hash_map, size_t bucket) { kv_ = hash_map->_pairs.data() + (int)bucket; }

                    const_iterator& operator++()
                    {
                        kv_ ++;
                        return *this;
                    }

                    const_iterator operator++(int)
                    {
                        auto cur = *this; kv_ ++;
                        return cur;
                    }

                    const_iterator& operator--()
                    {
                        kv_ --;
                        return *this;
                    }

                    const_iterator operator--(int)
                    {
                        auto cur = *this; kv_ --;
                        return cur;
                    }

                    const_reference operator*() const { return *kv_; }
                    const_pointer operator->() const { return kv_; }

                    bool operator == (const iterator& rhs) const { return kv_ == rhs.kv_; }
                    bool operator != (const iterator& rhs) const { return kv_ != rhs.kv_; }
                    bool operator == (const const_iterator& rhs) const { return kv_ == rhs.kv_; }
                    bool operator != (const const_iterator& rhs) const { return kv_ != rhs.kv_; }
                public:
                    const value_type* kv_;
            };

            constexpr float max_load_factor() const { return (1 << 27) / (float) mlf_; }
            constexpr size_t max_size() const { return (1ull << (sizeof(size_t) * 8 - 1)); }

            static constexpr bool is_triviall_destructable() { return !(std::is_trivially_destructible<KeyT>::value && std::is_trivially_destructible<ValueT>::value); }
            static constexpr bool is_copy_trivially() { return (std::is_trivially_copyable<KeyT>::value && std::is_trivially_copyable<ValueT>::value); }

            static inline KeyT get_key(value_type v) noexcept;

            void init(size_t bucket, float mlf = DEFAULT_LOAD_FACTOR);

            explicit BananHashTable(size_t bucket = 2, float mlf = DEFAULT_LOAD_FACTOR);
            BananHashTable(const BananHashTable& rhs);
            BananHashTable(BananHashTable&& rhs) noexcept;
            BananHashTable& operator=(const BananHashTable& rhs);
            BananHashTable& operator=(BananHashTable&& rhs) noexcept;

            ~BananHashTable();

            template<typename Con>
            bool operator == (const Con& rhs) const;

            void clone(const BananHashTable& rhs);
            void swap(BananHashTable& rhs);

            inline iterator first() { return {this, 0}; }
            inline iterator last() { return {this, num_filled_ - 1}; }

            inline value_type& front() { return pairs_[0]; }
            inline const value_type& front() const { return pairs_[0]; }
            inline value_type& back() { return pairs_[num_filled_ - 1]; }
            inline const value_type& back() const { return pairs_[num_filled_ - 1]; }

            inline void pop_front() { erase(begin()); }
            inline void pop_back() { erase(last()); }

            inline iterator begin() { return first(); }
            inline const_iterator cbegin() const { return first(); }
            inline const_iterator begin() const { return first(); }

            inline iterator end() { return {this, num_filled_}; }
            inline const_iterator cend() const { return {this, num_filled_}; }
            inline const_iterator end() const { return cend(); }

            // inline std::span<const value_type> values() const { return std::span<value_type>{pairs_}; }
            // inline std::span<const Index> index() const { return std::span<Index>{index_}; }

            inline size_t size() const { return num_filled_; }
            inline bool empty() const { return num_filled_ == 0; }
            inline size_t bucket_count() const { return num_buckets_; }

            inline float load_factor() const { return static_cast<float>(num_filled_) / (mask_ + 1); }

            inline HashT& hash_function() const { return hasher_; }
            inline EqT& key_eq() const { return eq_; }

            void max_load_factor(float mlf)
            {
                if (mlf < 0.992 && mlf > MIN_LOAD_FACTOR) {
                    mlf_ = (uint32_t)((1 << 27) / mlf);
                    if (num_buckets_ > 0) rehash(num_buckets_);
                }
            }

            template<typename K=KeyT>
            inline iterator find(const K& key) noexcept { return {this, find_filled_slot(key)}; }
            template<typename K=KeyT>
            inline const_iterator find(const K& key) const noexcept { return {this, find_filled_slot(key)}; }


            template<typename K=KeyT, typename V=ValueT>
            inline V& at(const K& key) requires (!std::is_void_v<V>)
            {
                const auto slot = find_filled_slot(key);
                return pairs_[slot].second;
            }

            template<typename K=KeyT, typename V=ValueT>
            inline const V& at(const K& key) const requires (!std::is_void_v<V>)
            {
                const auto slot = find_filled_slot(key);
                return pairs_[slot].second;
            }

            template<typename V = ValueT>
            inline const V& index(const uint32_t index) const requires (!std::is_void_v<V>) { return pairs_[index].second; }
            template<typename V = ValueT>
            inline V& index(const uint32_t index) requires (!std::is_void_v<V>) { return pairs_[index].second; }

            template<typename K=KeyT>
            inline bool contains(const K& key) const noexcept { return find_filled_slot(key) != num_filled_; }
            template<typename K=KeyT>
            inline size_t count(const K& key) const noexcept { return find_filled_slot(key) == num_filled_ ? 0 : 1; }

            template<typename K=KeyT>
            std::pair<iterator, iterator> equal_range(const K& key);
            void merge(BananHashTable& rhs);

            template<typename V = ValueT>
            bool try_get(const KeyT& key, V& val) const noexcept requires (!std::is_void_v<ValueT>);
            template<typename V = ValueT>
            V* try_get(const KeyT& key) const noexcept requires (!std::is_void_v<ValueT>);
            template<typename V = ValueT>
            bool try_set(const KeyT& key, const V& val) noexcept requires (!std::is_void_v<ValueT>);
            template<typename V = ValueT>
            bool try_set(const KeyT& key, V&& val) noexcept requires (!std::is_void_v<ValueT>);
            template<typename V = ValueT>
            V get_or_return_default(const KeyT& key) const noexcept requires (!std::is_void_v<ValueT>);

            std::pair<iterator, bool> do_insert(const value_type& value) noexcept;
            std::pair<iterator, bool> do_insert(value_type&& value) noexcept;

            template<typename K, typename V = ValueT>
            std::pair<iterator, bool> do_insert(K&& key, V&& val) noexcept requires (!std::is_void_v<ValueT>);
            template<typename K, typename V = ValueT>
            std::pair<iterator, bool> do_assign(K&& key, V&& val) noexcept requires (!std::is_void_v<ValueT>);

            std::pair<iterator, bool> insert(const value_type& p);
            std::pair<iterator, bool> insert(value_type && p);
            void insert(std::initializer_list<value_type> ilist);

            template <typename Iter>
            void insert(Iter first, Iter last);

            template<typename K, typename V = ValueT>
            size_t insert_unique(K&& key, V&& val) requires (!std::is_void_v<ValueT>);

            size_t insert_unique(value_type&& value);
            size_t insert_unique(const value_type& value);

            template <class... Args>
            std::pair<iterator, bool> emplace(Args&&... args) noexcept;
            template <class... Args>
            iterator emplace_hint(const_iterator hint, Args&&... args);
            template<class... Args>
            std::pair<iterator, bool> try_emplace(const KeyT& k, Args&&... args);
            template<class... Args>
            std::pair<iterator, bool> try_emplace(KeyT&& k, Args&&... args);
            template <class... Args>
            size_t emplace_unique(Args&&... args);

            template<typename V = ValueT>
            inline std::pair<iterator, bool> insert_or_assign(const KeyT& key, V&& val) requires (!std::is_void_v<ValueT>) { return do_assign(key, std::forward<ValueT>(val)); }
            template<typename V = ValueT>
            inline std::pair<iterator, bool> insert_or_assign(KeyT&& key, V&& val) requires (!std::is_void_v<ValueT>) { return do_assign(std::move(key), std::forward<ValueT>(val)); }

            template<typename V = ValueT>
            V set_get(const KeyT& key, const V& val) requires (!std::is_void_v<ValueT>);
            template<typename V = ValueT>
            V& operator[](const KeyT& key) noexcept requires (!std::is_void_v<ValueT>);
            template<typename V = ValueT>
            V& operator[](KeyT&& key) noexcept requires (!std::is_void_v<ValueT>);

            size_t erase(const KeyT& key) noexcept;
            iterator erase(const const_iterator& cit) noexcept;
            iterator erase(const_iterator first, const_iterator last) noexcept;

            template<typename Pred>
            size_t erase_if(Pred pred);

            void clearkv();
            void clear() noexcept;
            void shrink_to_fit(float min_factor = DEFAULT_LOAD_FACTOR / 4);
            bool reserve(uint64_t num_elems, bool force);

            static value_type* alloc_bucket(size_t num_buckets);
            static Index* alloc_index(size_t num_buckets);

            bool reserve(size_t required_buckets) noexcept;

            void rebuild(size_t num_buckets) noexcept;
            void rehash(uint64_t required_buckets);
            bool check_expand_need();

            static void prefetch_heap_block(char* ctrl);

            size_t slot_to_bucket(size_t slot) const noexcept;
            void erase_slot(size_t sbucket, size_t main_bucket) noexcept;
            size_t erase_bucket(size_t bucket, size_t main_bucket) noexcept;
            size_t find_slot_bucket(size_t slot, size_t& main_bucket) const;
            size_t find_filled_bucket(const KeyT& key, uint64_t key_hash) const noexcept;

            template<typename K=KeyT>
            size_t find_filled_slot(const K& key) const noexcept;
            template<typename K=KeyT>
            size_t find_or_allocate(const K& key, uint64_t key_hash) noexcept;

            size_t kickout_bucket(size_t kmain, size_t bucket) noexcept;
            size_t find_unique_bucket(uint64_t key_hash) noexcept;
            size_t find_empty_bucket(size_t bucket_from, uint32_t csize) noexcept;

            size_t find_last_bucket(size_t main_bucket) const;
            size_t find_prev_bucket(size_t main_bucket, size_t bucket) const;
            size_t hash_bucket(const KeyT& key) const noexcept;
            size_t hash_main(size_t bucket) const noexcept;

            inline uint64_t hash_key(const KeyT key) const { return hasher_(key); };
    };

    template <class KeyT, class ValueT>
    using BananHashMap = BananHashTable<KeyT, ValueT, BananHash<KeyT>, std::equal_to<KeyT>>;

    template <class KeyT>
    using BananHashSet = BananHashTable<KeyT, void, BananHash<KeyT>, std::equal_to<KeyT>>;
}

#include "hashtable.tpp"