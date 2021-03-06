#include <select_formaction.h>
#include <formatstring.h>
#include <view.h>
#include <config.h>
#include <utils.h>
#include <listformatter.h>

#include <sstream>
#include <cassert>

namespace newsbeuter {

/*
 * The select_formaction is used both for the "select tag" dialog
 * and the "select filter", as they do practically the same. That's
 * why there is the decision between SELECTTAG and SELECTFILTER on
 * a few places.
 */

select_formaction::select_formaction(view * vv, std::string formstr)
	: formaction(vv, formstr) { }

select_formaction::~select_formaction() { }

void select_formaction::handle_cmdline(const std::string& cmd) {
	unsigned int idx = 0;
	if (1==sscanf(cmd.c_str(),"%u",&idx)) {
		if (idx > 0 && idx <= ((type == SELECTTAG) ? tags.size() : filters.size())) {
			f->set("tagpos", utils::to_string<unsigned int>(idx - 1));
		}
	} else {
		formaction::handle_cmdline(cmd);
	}
}

void select_formaction::process_operation(operation op, bool /* automatic */, std::vector<std::string> * /* args */) {
	bool hardquit = false;
	switch (op) {
	case OP_QUIT:
		value = "";
		quit = true;
		break;
	case OP_HARDQUIT:
		value = "";
		hardquit = true;
		break;
	case OP_OPEN: {
		std::string tagposname = f->get("tagposname");
		unsigned int pos = utils::to_u(tagposname);
		if (tagposname.length() > 0) {
			switch (type) {
			case SELECTTAG: {
				if (pos < tags.size()) {
					value = tags[pos];
					quit = true;
				}
			}
			break;
			case SELECTFILTER: {
				if (pos < filters.size()) {
					value = filters[pos].second;
					quit = true;
				}
			}
			break;
			default:
				assert(0); // should never happen
			}
		}
	}
	break;
	default:
		break;
	}

	if (hardquit) {
		while (v->formaction_stack_size() > 0) {
			v->pop_current_formaction();
		}
	} else if (quit) {
		v->pop_current_formaction();
	}
}

void select_formaction::prepare() {
	if (do_redraw) {
		listformatter listfmt;
		unsigned int i=0;
		switch (type) {
		case SELECTTAG:
			for (auto tag : tags) {
				std::string tagstr = utils::strprintf("%4u  %s (%u)", i+1, tag.c_str(), v->get_ctrl()->get_feed_count_per_tag(tag));
				listfmt.add_line(tagstr, i);
				i++;
			}
			break;
		case SELECTFILTER:
			for (auto filter : filters) {
				std::string tagstr = utils::strprintf("%4u  %s", i+1, filter.first.c_str());
				listfmt.add_line(tagstr, i);
				i++;
			}
			break;
		default:
			assert(0);
		}
		f->modify("taglist", "replace_inner", listfmt.format_list());

		do_redraw = false;
	}
}

void select_formaction::init() {
	std::string title;
	do_redraw = true;
	quit = false;
	value = "";

	std::string viewwidth = f->get("taglist:w");
	unsigned int width = utils::to_u(viewwidth, 80);

	set_keymap_hints();

	fmtstr_formatter fmt;
	fmt.register_fmt('N', PROGRAM_NAME);
	fmt.register_fmt('V', PROGRAM_VERSION);

	switch (type) {
	case SELECTTAG:
		title = fmt.do_format(v->get_cfg()->get_configvalue("selecttag-title-format"), width);
		break;
	case SELECTFILTER:
		title = fmt.do_format(v->get_cfg()->get_configvalue("selectfilter-title-format"), width);
		break;
	default:
		assert(0); // should never happen
	}
	f->set("head", title);
}

keymap_hint_entry * select_formaction::get_keymap_hint() {
	static keymap_hint_entry hints_tag[] = {
		{ OP_QUIT, _("Cancel") },
		{ OP_OPEN, _("Select Tag") },
		{ OP_NIL, NULL }
	};
	static keymap_hint_entry hints_filter[] = {
		{ OP_QUIT, _("Cancel") },
		{ OP_OPEN, _("Select Filter") },
		{ OP_NIL, NULL }
	};
	switch (type) {
	case SELECTTAG:
		return hints_tag;
	case SELECTFILTER:
		return hints_filter;
	}
	return NULL;
}

std::string select_formaction::title() {
	switch (type) {
	case SELECTTAG:
		return _("Select Tag");
	case SELECTFILTER:
		return _("Select Filter");
	default:
		return "";
	}
}


}
