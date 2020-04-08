#include <utility>


#include "widgets/ui_tree_list.h"

#include "widgets/ui_image.h"
#include "widgets/ui_label.h"
using namespace Halley;


UITreeList::UITreeList(String id, UIStyle style)
	: UIList(std::move(id), std::move(style))
{
}

void UITreeList::addTreeItem(const String& id, const String& parentId, const LocalisedString& label)
{
	auto listItem = std::make_shared<UIListItem>(id, *this, style.getSubStyle("item"), int(getNumberOfItems()), style.getBorder("extraMouseBorder"));

	// Controls
	const auto treeControls = std::make_shared<UITreeListControls>(id, style.getSubStyle("controls"));
	listItem->add(treeControls, 0, {}, UISizerFillFlags::Fill);

	// Icon
	//auto icon = std::make_shared<UIImage>(style.getSubStyle("controls").getSprite("element"));
	//listItem->add(icon, 0, {}, UISizerAlignFlags::Centre);

	// Label
	auto labelWidget = std::make_shared<UILabel>(id + "_label", style.getTextRenderer("label"), label);
	if (style.hasTextRenderer("selectedLabel")) {
		labelWidget->setSelectable(style.getTextRenderer("label"), style.getTextRenderer("selectedLabel"));
	}
	if (style.hasTextRenderer("disabledStyle")) {
		labelWidget->setDisablable(style.getTextRenderer("label"), style.getTextRenderer("disabledStyle"));
	}
	listItem->add(labelWidget, 0, style.getBorder("labelBorder"), UISizerFillFlags::Fill);

	// Logical item
	auto treeItem = UITreeListItem(id, listItem, treeControls);
	auto& parentItem = getItemOrRoot(parentId);
	parentItem.addChild(std::move(treeItem));

	addItem(listItem, Vector4f(), UISizerAlignFlags::Left | UISizerFillFlags::FillVertical);
}

void UITreeList::update(Time t, bool moved)
{
	UIList::update(t, moved);
	root.updateTree();
}

UITreeListItem& UITreeList::getItemOrRoot(const String& id)
{
	const auto res = root.tryFindId(id);
	if (res) {
		return *res;
	}
	return root;
}

UITreeListControls::UITreeListControls(String id, UIStyle style)
	: UIWidget(std::move(id), Vector2f(), UISizer(UISizerType::Horizontal, 0))
	, style(std::move(style))
{
	setupUI();
}

float UITreeListControls::updateGuides(const std::vector<int>& itemsLeftPerDepth, bool hasChildren)
{
	if (waitingConstruction || itemsLeftPerDepth.size() != lastDepth) {
		clear();
		guides.clear();
		lastDepth = itemsLeftPerDepth.size();

		auto getSprite = [&](size_t depth) -> Sprite
		{
			const bool leaf = depth == itemsLeftPerDepth.size();
			if (leaf) {
				return style.getSprite("leaf");
			} else {
				const bool deepest = depth == itemsLeftPerDepth.size() - 1;
				const auto left = itemsLeftPerDepth[depth];
				if (deepest) {
					if (left == 1) {
						return style.getSprite("guide_l");
					} else {
						return style.getSprite("guide_t");
					}
				} else {
					if (left == 1) {
						return Sprite().setSize(Vector2f(22, 22));
					} else {
						return style.getSprite("guide_i");
					}
				}
			}
		};
		
		for (size_t i = 1; i < itemsLeftPerDepth.size() + (hasChildren ? 0 : 1); ++i) {
			guides.push_back(std::make_shared<UIImage>(getSprite(i)));
			add(guides.back(), 0, Vector4f(0, -1, 0, 0));
		}

		if (hasChildren) {
			expandButton = std::make_shared<UIButton>("expand", style.getSubStyle("expandButton"));
			collapseButton = std::make_shared<UIButton>("collapse", style.getSubStyle("collapseButton"));
			expandButton->setActive(false);
			add(expandButton, 0, Vector4f(), UISizerAlignFlags::Centre);
			add(collapseButton, 0, Vector4f(), UISizerAlignFlags::Centre);
		}

		totalIndent = getLayoutMinimumSize(false).x;
		waitingConstruction = false;
	}

	return totalIndent;
}

void UITreeListControls::setupUI()
{
	setHandle(UIEventType::ButtonClicked, "expand", [=] (const UIEvent& event)
	{
		expandButton->setActive(false);
		collapseButton->setActive(true);
	});
	setHandle(UIEventType::ButtonClicked, "collapse", [=](const UIEvent& event)
	{
		expandButton->setActive(true);
		collapseButton->setActive(false);
	});
}

UITreeListItem::UITreeListItem() = default;

UITreeListItem::UITreeListItem(String id, std::shared_ptr<UIListItem> listItem, std::shared_ptr<UITreeListControls> treeControls)
	: id(std::move(id))
	, listItem(std::move(listItem))
	, treeControls(std::move(treeControls))
{}

UITreeListItem* UITreeListItem::tryFindId(const String& id)
{
	if (id == this->id) {
		return this;
	}

	for (auto& c: children) {
		const auto res = c.tryFindId(id);
		if (res) {
			return res;
		}
	}

	return nullptr;
}

void UITreeListItem::addChild(UITreeListItem item)
{
	children.emplace_back(std::move(item));
}

void UITreeListItem::updateTree()
{
	std::vector<int> itemsLeftPerDepth;
	doUpdateTree(itemsLeftPerDepth);
}

void UITreeListItem::doUpdateTree(std::vector<int>& itemsLeftPerDepth)
{
	if (listItem && treeControls) {
		const float totalIndent = treeControls->updateGuides(itemsLeftPerDepth, !children.empty());
		listItem->setClickableInnerBorder(Vector4f(totalIndent, 0, 0, 0));
	}

	itemsLeftPerDepth.push_back(int(children.size()));

	for (auto& c: children) {
		c.doUpdateTree(itemsLeftPerDepth);
		itemsLeftPerDepth.back()--;
	}

	itemsLeftPerDepth.pop_back();
}