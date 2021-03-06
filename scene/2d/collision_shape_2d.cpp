/*************************************************************************/
/*  collision_shape_2d.cpp                                               */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2017 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2017 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#include "collision_shape_2d.h"
#include "collision_object_2d.h"
#include "scene/resources/capsule_shape_2d.h"
#include "scene/resources/circle_shape_2d.h"
#include "scene/resources/concave_polygon_shape_2d.h"
#include "scene/resources/convex_polygon_shape_2d.h"
#include "scene/resources/rectangle_shape_2d.h"
#include "scene/resources/segment_shape_2d.h"
#include "scene/resources/shape_line_2d.h"

void CollisionShape2D::_shape_changed() {

	update();
}

void CollisionShape2D::_notification(int p_what) {

	switch (p_what) {

		case NOTIFICATION_PARENTED: {

			parent = get_parent()->cast_to<CollisionObject2D>();
			if (parent) {
				owner_id = parent->create_shape_owner(this);
				if (shape.is_valid()) {
					parent->shape_owner_add_shape(owner_id, shape);
				}
				parent->shape_owner_set_transform(owner_id, get_transform());
				parent->shape_owner_set_disabled(owner_id, disabled);
				parent->shape_owner_set_one_way_collision(owner_id, one_way_collision);
			}

			/*if (get_tree()->is_editor_hint()) {
				//display above all else
				set_z_as_relative(false);
				set_z(VS::CANVAS_ITEM_Z_MAX - 1);
			}*/

		} break;
		case NOTIFICATION_LOCAL_TRANSFORM_CHANGED: {

			if (parent) {
				parent->shape_owner_set_transform(owner_id, get_transform());
			}

		} break;
		case NOTIFICATION_UNPARENTED: {
			if (parent) {
				parent->remove_shape_owner(owner_id);
			}
			owner_id = 0;
			parent = NULL;
		} break;
		/*
		case NOTIFICATION_TRANSFORM_CHANGED: {

			if (!is_inside_scene())
				break;
			_update_parent();

		} break;*/
		case NOTIFICATION_DRAW: {

			if (!get_tree()->is_editor_hint() && !get_tree()->is_debugging_collisions_hint()) {
				break;
			}

			if (!shape.is_valid()) {
				break;
			}

			rect = Rect2();

			Color draw_col = get_tree()->get_debug_collisions_color();
			if (disabled) {
				float g = draw_col.gray();
				draw_col.r = g;
				draw_col.g = g;
				draw_col.b = g;
			}
			shape->draw(get_canvas_item(), draw_col);

			rect = shape->get_rect();
			rect = rect.grow(3);

			if (one_way_collision) {
				Color dcol = get_tree()->get_debug_collisions_color(); //0.9,0.2,0.2,0.4);
				dcol.a = 1.0;
				Vector2 line_to(0, 20);
				draw_line(Vector2(), line_to, dcol, 3);
				Vector<Vector2> pts;
				float tsize = 8;
				pts.push_back(line_to + (Vector2(0, tsize)));
				pts.push_back(line_to + (Vector2(0.707 * tsize, 0)));
				pts.push_back(line_to + (Vector2(-0.707 * tsize, 0)));
				Vector<Color> cols;
				for (int i = 0; i < 3; i++)
					cols.push_back(dcol);

				draw_primitive(pts, cols, Vector<Vector2>()); //small arrow
			}
		} break;
	}
}

void CollisionShape2D::set_shape(const Ref<Shape2D> &p_shape) {

	if (shape.is_valid())
		shape->disconnect("changed", this, "_shape_changed");
	shape = p_shape;
	update();
	if (parent) {
		parent->shape_owner_clear_shapes(owner_id);
		if (shape.is_valid()) {
			parent->shape_owner_add_shape(owner_id, shape);
		}
	}

	if (shape.is_valid())
		shape->connect("changed", this, "_shape_changed");

	update_configuration_warning();
}

Ref<Shape2D> CollisionShape2D::get_shape() const {

	return shape;
}

Rect2 CollisionShape2D::get_item_rect() const {

	return rect;
}

String CollisionShape2D::get_configuration_warning() const {

	if (!get_parent()->cast_to<CollisionObject2D>()) {
		return TTR("CollisionShape2D only serves to provide a collision shape to a CollisionObject2D derived node. Please only use it as a child of Area2D, StaticBody2D, RigidBody2D, KinematicBody2D, etc. to give them a shape.");
	}

	if (!shape.is_valid()) {
		return TTR("A shape must be provided for CollisionShape2D to function. Please create a shape resource for it!");
	}

	return String();
}

void CollisionShape2D::set_disabled(bool p_disabled) {
	disabled = p_disabled;
	update();
	if (parent) {
		parent->shape_owner_set_disabled(owner_id, p_disabled);
	}
}

bool CollisionShape2D::is_disabled() const {
	return disabled;
}

void CollisionShape2D::set_one_way_collision(bool p_enable) {
	one_way_collision = p_enable;
	update();
	if (parent) {
		parent->shape_owner_set_one_way_collision(owner_id, p_enable);
	}
}

bool CollisionShape2D::is_one_way_collision_enabled() const {

	return one_way_collision;
}

void CollisionShape2D::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_shape", "shape"), &CollisionShape2D::set_shape);
	ClassDB::bind_method(D_METHOD("get_shape"), &CollisionShape2D::get_shape);
	ClassDB::bind_method(D_METHOD("set_disabled", "disabled"), &CollisionShape2D::set_disabled);
	ClassDB::bind_method(D_METHOD("is_disabled"), &CollisionShape2D::is_disabled);
	ClassDB::bind_method(D_METHOD("set_one_way_collision", "enabled"), &CollisionShape2D::set_one_way_collision);
	ClassDB::bind_method(D_METHOD("is_one_way_collision_enabled"), &CollisionShape2D::is_one_way_collision_enabled);
	ClassDB::bind_method(D_METHOD("_shape_changed"), &CollisionShape2D::_shape_changed);

	ADD_PROPERTYNZ(PropertyInfo(Variant::OBJECT, "shape", PROPERTY_HINT_RESOURCE_TYPE, "Shape2D"), "set_shape", "get_shape");
	ADD_PROPERTYNZ(PropertyInfo(Variant::BOOL, "disabled"), "set_disabled", "is_disabled");
	ADD_PROPERTYNZ(PropertyInfo(Variant::BOOL, "one_way_collision"), "set_one_way_collision", "is_one_way_collision_enabled");
}

CollisionShape2D::CollisionShape2D() {

	rect = Rect2(-Point2(10, 10), Point2(20, 20));
	set_notify_local_transform(true);
	owner_id = 0;
	parent = NULL;
	disabled = false;
	one_way_collision = false;
}
