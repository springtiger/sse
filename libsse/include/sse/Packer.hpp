/**
 * StepSlicerEngine
 * Copyright (C) 2020 Karl Nilsson
 *
 * This program is free software: you can redistribute it and/or modify
 * under the terms of the GNU Affero General Public License as published by
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

/**
 * @file Packer.hpp
 * @brief Packs objects into a rectangular bin
 *
 * This contains the prototypes for the Packer class
 *
 * @author Karl Nilsson
 * @bug fixed/static offset dimension between objects, Bnd_Box::SetGap()
 * @bug bounds check: ensure objects don't have infinite dimensions
 * @bug bounds check: ensure object list isn't too big, otherwise will
 * run out of stack space when destroying tree
 */

#pragma once

#include <algorithm>
#include <exception>
#include <memory>
#include <utility>
#include <vector>

#include <sse/Object.hpp>

#include <spdlog/spdlog.h>

/**
 * TODO:
 * allow for configurable/dynamic OFFSET space between footprint rectangles
 * (i.e. expand all rectangles individually, based on brim, if applicable.
 * keep in mind, brim may not expand footprint, i.e. brim for a sphere)
 */

namespace sse {

/**
 * @class Packer
 * @brief Pack objects into a rectangular bin, based on their XY bounding box.
 *
 * Construct a binary tree of Nodes, growing the bin and tree to fit objects. A
 * few heuristics are employed to minimize wasted space, ensuring the resulting
 * bin has close to equal dimensions (i.e. roughly a square).
 *
 */
class Packer {

public:
  /**
   * @brief Packer constructor
   * @param objects List of objects to pack
   */
  explicit Packer(std::vector<std::shared_ptr<Object>> objects);

  /**
   * @brief Calculate an optimized rectangular bin for the objects
   * @return Dimensions of resulting bin
   * @throws std::runtime Thrown if unable to grow bin properly
   */
  std::pair<double, double> pack();

  /**
   * @brief Move all objects to their new position on the buildplate
   * @param offset_x X offset of bin with respect to buildplate origin
   * @param offset_y Y offset of bin with respect to buildplate origin
   */
  void arrange(double offset_x, double offset_y) const;

private:
  /**
   * @struct Node
   * @brief Binary tree Node
   *
   * This struct describes a binary tree Node that corresponds to a rectangle in
   * the cartesian plane. The tree is used to pack objects into a rectangular
   * bin. For the purposes of this class, width is a dimension in the X axis,
   * and length in the Y axis.
   */
  struct Node {
    //! shorthand for shared_ptr
    using node_ptr = std::unique_ptr<Node>;
    //! X position
    const double x;
    //! Y position
    const double y;
    //! node width
    const double width;
    //! node length
    const double length;
    //! up child node
    node_ptr up{nullptr};
    //! right child node
    node_ptr right{nullptr};
    //! object contained in this node
    Object *object{nullptr};

    /**
     * @brief Node constructor
     * @param y X position
     * @param y Y position
     * @param w Width, X axis
     * @param l Length, Y axis
     */
    Node(double x, double y, double w, double l, node_ptr up=nullptr, node_ptr right=nullptr)
        : x(x), y(y), width(w), length(l), up(std::move(up)), right(std::move(right)) {
    }

    /**
     * @brief Add object to node, then make child nodes out of leftovers
     * @param o Object to add
     */
    void add_object(Object *o) {
      object = o;
      up = std::make_unique<Node>(x, y + object->length(), width,
                                  length - object->length());
      right = std::make_unique<Node>(x + object->length(), y,
                                     width - object->width(), length);
    }

    /**
     * @brief Check to see if object will fit in this node
     * @param o Target object
     * @return Whether object fits in node
     */
    inline bool fits(const Object *o) const {
      return (o->length() <= length) && (o->width() <= width);
    }

    /**
     * @brief Does this node contain an object?
     * @return Whether node contains an object
     */
    inline bool full() const { return object != nullptr; }

    /**
     * @brief Is this node a leaf?
     * @return Whether node is a leaf
     */
    inline bool leaf() const { return up == nullptr; }

    /**
     * @brief Stream overload
     * @param out Out stream
     * @param node Target node
     * @return stream
     */
    friend std::ostream& operator<<(std::ostream& out, Node& node) {
      out << node.x << "," << node.y << " " << node.width << "x" << node.length;
      return out;
    }

  }; // end Node definition

  /**
   * @brief Search the tree for a suitable node to hold an object
   * @param o The object to insert
   * @return pointer to a suitable node, nullptr otherwise
   */
  Node *insert_search(Node &node, const Object *o) const;

  /**
   * @brief Grow the bin in the +Y direction
   * @param width Width requested
   * @param length Length requested
   * @return A new Node, big enough to fit the space required
   */
  Node *grow_up(double width, double length);

  /**
   * @brief Grow the bin in the +X direction
   * @param width Width requested
   * @param length Length requested
   * @return A new Node, big enough to fit the space required
   */
  Node *grow_right(double width, double length);

  /**
   * @brief Translate an object to its new position, then recurse to children
   * @param node Object to translate
   * @param offset_x X offset of bin with respect to buildplate origin
   * @param offset_y Y offset of bin with respect to buildplate origin
   */
  void translate(const Node &node, const double offset_x,
                 const double offset_y) const;

  //! list of objects to pack
  std::vector<std::shared_ptr<Object>> objects;
  //! root node of binary tree
  std::unique_ptr<Node> root;
};

} // namespace sse
