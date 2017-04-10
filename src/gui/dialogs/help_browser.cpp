/*
	Copyright (C) 2017 - 2024
	by Charles Dang <exodia339@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/help_browser.hpp"

#include "game_config_manager.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/multi_page.hpp"
#include "gui/widgets/tree_view.hpp"
#include "gui/widgets/tree_view_node.hpp"
#include "gui/widgets/window.hpp"

#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#endif


namespace gui2::dialogs
{

REGISTER_DIALOG(help_browser)

help_browser::help_browser()
	: modal_dialog(window_id())
	, initial_topic_("introduction")
	, help_cfg_(game_config_manager::get()->game_config().mandatory_child("help"))
{
}

void help_browser::pre_show(window& window)
{
	tree_view& topic_tree = find_widget<tree_view>(&window, "topic_tree", false);

	connect_signal_notify_modified(topic_tree,
		std::bind(&help_browser::on_topic_select, this));

	window.keyboard_capture(&topic_tree);

	const config toc = help_cfg_.mandatory_child("toplevel");

	for(const std::string& section : utils::split(toc["sections"])) {
		add_topic(help_cfg_.find_child("section", "id", section).value(), true);
	}

	for(const std::string& topic : utils::split(toc["topics"])) {
		add_topic(help_cfg_.find_child("topic", "id", topic).value(), false);
	}

	on_topic_select();
}

void help_browser::add_topic(const config& topic, bool expands, tree_view_node*) {
	tree_view& topic_tree = find_widget<tree_view>(this, "topic_tree", false);
	widget_data data;
	widget_item item;

	item["label"] = topic["title"];
	data.emplace("topic_name", item);

	item.clear();
	item["label"] = expands ? "help/closed_section.png" : "help/topic.png";
	data.emplace("topic_icon", item);

	topic_tree.add_node("topic", data).set_id(std::string(expands ? "+" : "-") + topic["id"]);
}

void help_browser::on_topic_select()
{
	multi_page& topic_pages = find_widget<multi_page>(this, "topic_text_pages", false);
	tree_view& topic_tree = find_widget<tree_view>(this, "topic_tree", false);

	if(topic_tree.empty()) {
		return;
	}

	assert(topic_tree.selected_item());
	std::string topic_id = topic_tree.selected_item()->id();

	if(topic_id.empty()) {
		return;
	}

	if(topic_id[0] == '+') {
		topic_id.replace(topic_id.begin(), topic_id.begin() + 1, 2, '.');
	} else {
		topic_id.erase(topic_id.begin());
	}

	auto iter = parsed_pages_.find(topic_id);
	if(iter == parsed_pages_.end()) {
		auto topic = help_cfg_.find_child("topic", "id", topic_id);
		if(!topic.has_value()) {
			return;
		}

		widget_data data;
		widget_item item;

		item["label"] = topic["text"];
		data.emplace("topic_text", item);

		parsed_pages_.emplace(topic_id, topic_pages.get_page_count());
		topic_pages.add_page(data);
		invalidate_layout();
	}

	const unsigned topic_i = parsed_pages_.at(topic_id);
	topic_pages.select_page(topic_i);

}

} // namespace dialogs
