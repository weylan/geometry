// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2016 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_AGGREGATE_OPERATIONS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_AGGREGATE_OPERATIONS_HPP

#include <set>

#include <boost/geometry/algorithms/detail/overlay/sort_by_side.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay { namespace sort_by_side
{

struct ring_with_direction
{
    ring_identifier ring_id;
    direction_type direction;

    std::size_t turn_index;
    int operation_index;
    operation_type operation;
    signed_size_type region_id;
    bool isolated;

    inline bool operator<(ring_with_direction const& other) const
    {
        return this->ring_id != other.ring_id
                ? this->ring_id < other.ring_id
                : this->direction < other.direction;
    }

    ring_with_direction()
        : direction(dir_unknown)
        , turn_index(-1)
        , operation_index(0)
        , operation(operation_none)
        , region_id(-1)
        , isolated(false)
    {}
};

struct rank_with_rings
{
    std::size_t rank;
    std::set<ring_with_direction> rings;

    rank_with_rings()
        : rank(0)
    {
    }

    inline bool all_equal(direction_type dir_type) const
    {
        for (std::set<ring_with_direction>::const_iterator it = rings.begin();
             it != rings.end(); ++it)
        {
            if (it->direction != dir_type)
            {
                return false;
            }
        }
        return true;
    }

    inline bool all_to() const
    {
        return all_equal(sort_by_side::dir_to);
    }

    inline bool all_from() const
    {
        return all_equal(sort_by_side::dir_from);
    }

    inline bool is_c_i() const
    {
        bool has_c = false;
        bool has_i = false;
        for (std::set<ring_with_direction>::const_iterator it = rings.begin();
             it != rings.end(); ++it)
        {
            const ring_with_direction& rwd = *it;
            switch(rwd.operation)
            {
                case operation_continue : has_c = true; break;
                case operation_intersection : has_i = true; break;
                default : return false;
            }
        }
        return has_c && has_i;
    }

    inline bool is_isolated() const
    {
        for (std::set<ring_with_direction>::const_iterator it = rings.begin();
             it != rings.end(); ++it)
        {
            const ring_with_direction& rwd = *it;
            if (! rwd.isolated)
            {
                return false;
            }
        }
        return true;
    }

    template <typename Turns>
    inline bool traversable(Turns const& turns) const
    {
        typedef typename boost::range_value<Turns>::type turn_type;
        typedef typename turn_type::turn_operation_type turn_operation_type;

        for (std::set<ring_with_direction>::const_iterator it = rings.begin();
             it != rings.end(); ++it)
        {
            const ring_with_direction& rwd = *it;
            turn_type const& turn = turns[rwd.turn_index];
            turn_operation_type const& op = turn.operations[rwd.operation_index];
            if (op.visited.finalized())
            {
                return false;
            }
        }
        return true;
    }

};

template <typename Sbs, typename Turns>
inline void aggregate_operations(Sbs const& sbs, std::vector<rank_with_rings>& aggregation,
                                 Turns const& turns)
{
    typedef typename boost::range_value<Turns>::type turn_type;
    typedef typename turn_type::turn_operation_type turn_operation_type;

    aggregation.clear();
    for (std::size_t i = 0; i < sbs.m_ranked_points.size(); i++)
    {
        typename Sbs::rp const& ranked_point = sbs.m_ranked_points[i];

        turn_type const& turn = turns[ranked_point.turn_index];

        turn_operation_type const& op = turn.operations[ranked_point.operation_index];

        if (! (op.operation == operation_intersection || op.operation == operation_continue))
        {
            // Don't consider union/blocked (aggregate is only used for intersections)
            continue;
        }

        if (aggregation.empty() || aggregation.back().rank != ranked_point.rank)
        {
            rank_with_rings current;
            current.rank = ranked_point.rank;
            aggregation.push_back(current);
        }

        ring_with_direction rwd;
        segment_identifier const& sid = ranked_point.seg_id;

        rwd.ring_id = ring_identifier(sid.source_index, sid.multi_index, sid.ring_index);
        rwd.direction = ranked_point.direction;
        rwd.turn_index = ranked_point.turn_index;
        rwd.operation_index = ranked_point.operation_index;
        rwd.operation = op.operation;
        rwd.region_id = op.enriched.region_id;
        rwd.isolated = op.enriched.isolated;

        aggregation.back().rings.insert(rwd);
    }
}


}}} // namespace detail::overlay::sort_by_side
#endif //DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_AGGREGATE_OPERATIONS_HPP
