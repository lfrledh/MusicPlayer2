﻿#include "stdafx.h"
#include "UserUi.h"

CUserUi::CUserUi(UIData& ui_data, CWnd* pMainWnd, const std::wstring& xml_path)
    : CPlayerUIBase(ui_data, pMainWnd), m_xml_path(xml_path)
{
    LoadUi();
}


CUserUi::~CUserUi()
{
}

void CUserUi::SetIndex(int index)
{
    m_index = index;
}

bool CUserUi::IsIndexValid() const
{
    return m_index != INT_MAX;
}

void CUserUi::_DrawInfo(CRect draw_rect, bool reset)
{
    std::shared_ptr<UiElement::Element> draw_element = GetCurrentUiType();
    if (draw_element != nullptr)
    {
        if (m_ui_data.full_screen)  //全屏模式下，最外侧的边距需要加宽
        {
            draw_rect.DeflateRect(EdgeMargin(true), EdgeMargin(false));
        }
        draw_element->SetRect(draw_rect);
        draw_element->Draw(this);
        //绘制音量调整按钮
        DrawVolumnAdjBtn();
    }
    //绘制右上角图标
    DrawTopRightIcons(true);

    //全屏模式时在右上角绘制时间
    if (m_ui_data.full_screen)
    {
        DrawCurrentTime();
    }
    m_draw_data.thumbnail_rect = draw_rect;
}

std::shared_ptr<UiElement::Element> CUserUi::GetCurrentUiType()
{
    std::shared_ptr<UiElement::Element> draw_element;
    //根据不同的窗口大小选择不同的界面元素的根节点绘图
    auto ui_size = GetUiSize();
    //<ui type="small">
    if (ui_size == CPlayerUIBase::UiSize::SMALL)
    {
        if (m_root_ui_small != nullptr)
            draw_element = m_root_ui_small;
        else if (m_root_ui_narrow != nullptr)
            draw_element = m_root_ui_narrow;
        else
            draw_element = m_root_default;
    }
    //<ui type="big">
    else if (ui_size == CPlayerUIBase::UiSize::BIG)
    {
        if (m_root_ui_big != nullptr)
            draw_element = m_root_ui_big;
        else
            draw_element = m_root_default;
    }
    //<ui type="narrow">
    else
    {
        if (m_root_ui_narrow != nullptr)
            draw_element = m_root_ui_narrow;
        else
            draw_element = m_root_default;
    }
    return draw_element;
}

CString CUserUi::GetUIName()
{
    return m_ui_name.c_str();
}

bool CUserUi::LButtonUp(CPoint point)
{
    if (!CPlayerUIBase::LButtonUp(point))
    {
        const auto& stack_elements{ GetStackElements() };
        for (const auto& element : stack_elements)
        {
            UiElement::StackElement* stack_element = dynamic_cast<UiElement::StackElement*>(element.get());
            if (stack_element != nullptr)
            {
                bool pressed = stack_element->indicator.pressed;
                stack_element->indicator.pressed = false;

                if ((pressed && stack_element->indicator.rect.PtInRect(point) && stack_element->indicator.enable)
                    || (stack_element->ckick_to_switch && stack_element->GetRect().PtInRect(point)))
                {
                    m_draw_data.lyric_rect.SetRectEmpty();
                    stack_element->SwitchDisplay();
                    return true;
                }
            }
        }
    }
    return false;
}

void CUserUi::LButtonDown(CPoint point)
{
    CPlayerUIBase::LButtonDown(point);
    auto& stack_elements{ GetStackElements() };
    for (auto& element : stack_elements)
    {
        UiElement::StackElement* stack_element{ dynamic_cast<UiElement::StackElement*>(element.get()) };
        if (stack_element != nullptr && stack_element->indicator.enable && stack_element->indicator.rect.PtInRect(point) != FALSE)
            stack_element->indicator.pressed = true;
    }
}

void CUserUi::MouseMove(CPoint point)
{
    auto& stack_elements{ GetStackElements() };
    for (auto& element : stack_elements)
    {
        UiElement::StackElement* stack_element{ dynamic_cast<UiElement::StackElement*>(element.get()) };
        if (stack_element != nullptr && stack_element->indicator.enable)
            stack_element->indicator.hover = (stack_element->indicator.rect.PtInRect(point) != FALSE);
    }

    CPlayerUIBase::MouseMove(point);
}

int CUserUi::GetUiIndex()
{
    return m_index;
}

std::shared_ptr<UiElement::Element> CUserUi::BuildUiElementFromXmlNode(tinyxml2::XMLElement* xml_node)
{
    CElementFactory factory;
    //获取节点名称
    std::string item_name = CTinyXml2Helper::ElementName(xml_node);
    //根据节点名称创建ui元素
    std::shared_ptr<UiElement::Element> element = factory.CreateElement(item_name);
    if (element != nullptr)
    {
        static UiElement::Element* current_build_ui_element{};      //正在创建ui元素
        if (item_name == "ui")
            current_build_ui_element = element.get();

        element->name = item_name;
        //设置元素的基类属性
        std::string str_x = CTinyXml2Helper::ElementAttribute(xml_node, "x");
        std::string str_y = CTinyXml2Helper::ElementAttribute(xml_node, "y");
        std::string str_proportion = CTinyXml2Helper::ElementAttribute(xml_node, "proportion");
        std::string str_width = CTinyXml2Helper::ElementAttribute(xml_node, "width");
        std::string str_height = CTinyXml2Helper::ElementAttribute(xml_node, "height");
        std::string str_max_width = CTinyXml2Helper::ElementAttribute(xml_node, "max-width");
        std::string str_max_height = CTinyXml2Helper::ElementAttribute(xml_node, "max-height");
        std::string str_min_width = CTinyXml2Helper::ElementAttribute(xml_node, "min-width");
        std::string str_min_height = CTinyXml2Helper::ElementAttribute(xml_node, "min-height");
        std::string str_margin = CTinyXml2Helper::ElementAttribute(xml_node, "margin");
        std::string str_margin_left = CTinyXml2Helper::ElementAttribute(xml_node, "margin-left");
        std::string str_margin_right = CTinyXml2Helper::ElementAttribute(xml_node, "margin-right");
        std::string str_margin_top = CTinyXml2Helper::ElementAttribute(xml_node, "margin-top");
        std::string str_margin_bottom = CTinyXml2Helper::ElementAttribute(xml_node, "margin-bottom");
        std::string str_hide_width = CTinyXml2Helper::ElementAttribute(xml_node, "hide-width");
        std::string str_hide_height = CTinyXml2Helper::ElementAttribute(xml_node, "hide-height");
        if (!str_x.empty())
            element->x.FromString(str_x);
        if (!str_y.empty())
            element->y.FromString(str_y);
        if (!str_proportion.empty())
            element->proportion = max(atoi(str_proportion.c_str()), 1);
        if (!str_width.empty())
            element->width.FromString(str_width);
        if (!str_height.empty())
            element->height.FromString(str_height);
        if (!str_max_width.empty())
            element->max_width.FromString(str_max_width);
        if (!str_max_height.empty())
            element->max_height.FromString(str_max_height);
        if (!str_min_width.empty())
            element->min_width.FromString(str_min_width);
        if (!str_min_height.empty())
            element->min_height.FromString(str_min_height);
        
        if (!str_margin.empty())
        {
            element->margin_left.FromString(str_margin);
            element->margin_right.FromString(str_margin);
            element->margin_top.FromString(str_margin);
            element->margin_bottom.FromString(str_margin);
        }
        if (!str_margin_left.empty())
            element->margin_left.FromString(str_margin_left);
        if (!str_margin_right.empty())
            element->margin_right.FromString(str_margin_right);
        if (!str_margin_top.empty())
            element->margin_top.FromString(str_margin_top);
        if (!str_margin_bottom.empty())
            element->margin_bottom.FromString(str_margin_bottom);

        if (!str_hide_width.empty())
            element->hide_width.FromString(str_hide_width);
        if (!str_hide_height.empty())
            element->hide_height.FromString(str_hide_height);

        //根据节点的类型设置元素独有的属性
        //按钮
        if (item_name == "button")
        {
            UiElement::Button* button = dynamic_cast<UiElement::Button*>(element.get());
            if (button != nullptr)
            {
                std::string str_key = CTinyXml2Helper::ElementAttribute(xml_node, "key");   //按钮的类型
                button->FromString(str_key);
                std::string str_big_icon = CTinyXml2Helper::ElementAttribute(xml_node, "bigIcon");
                button->big_icon = CTinyXml2Helper::StringToBool(str_big_icon.c_str());
            }
        }
        else if (item_name == "rectangle")
        {
            UiElement::Rectangle* rectangle = dynamic_cast<UiElement::Rectangle*>(element.get());
            if (rectangle != nullptr)
            {
                std::string str_no_corner_radius = CTinyXml2Helper::ElementAttribute(xml_node, "no_corner_radius");
                rectangle->no_corner_radius = CTinyXml2Helper::StringToBool(str_no_corner_radius.c_str());
                std::string str_theme_color = CTinyXml2Helper::ElementAttribute(xml_node, "theme_color");
                if (!str_theme_color.empty())
                    rectangle->theme_color = CTinyXml2Helper::StringToBool(str_theme_color.c_str());
            }
        }
        //文本
        else if (item_name == "text")
        {
            UiElement::Text* text = dynamic_cast<UiElement::Text*>(element.get());
            if (text != nullptr)
            {
                //text
                std::string str_text = CTinyXml2Helper::ElementAttribute(xml_node, "text");
                text->text = CCommon::StrToUnicode(str_text, CodeType::UTF8_NO_BOM);
                //alignment
                std::string str_alignment = CTinyXml2Helper::ElementAttribute(xml_node, "alignment");
                if (str_alignment == "left")
                    text->align = Alignment::LEFT;
                else if (str_alignment == "right")
                    text->align = Alignment::RIGHT;
                else if (str_alignment == "center")
                    text->align = Alignment::CENTER;
                //style
                std::string str_style = CTinyXml2Helper::ElementAttribute(xml_node, "style");
                if (str_style == "static")
                    text->style = UiElement::Text::Static;
                else if (str_style == "scroll")
                    text->style = UiElement::Text::Scroll;
                else if (str_style == "scroll2")
                    text->style = UiElement::Text::Scroll2;
                //type
                std::string str_type = CTinyXml2Helper::ElementAttribute(xml_node, "type");
                if (str_type == "userDefine")
                    text->type = UiElement::Text::UserDefine;
                else if (str_type == "title")
                    text->type = UiElement::Text::Title;
                else if (str_type == "artist")
                    text->type = UiElement::Text::Artist;
                else if (str_type == "album")
                    text->type = UiElement::Text::Album;
                else if (str_type == "artist_title")
                    text->type = UiElement::Text::ArtistTitle;
                else if (str_type == "format")
                    text->type = UiElement::Text::Format;
                else if (str_type == "play_time")
                    text->type = UiElement::Text::PlayTime;
                //font_size
                std::string str_font_size = CTinyXml2Helper::ElementAttribute(xml_node, "font_size");
                text->font_size = atoi(str_font_size.c_str());
                if (text->font_size == 0)
                    text->font_size = 9;
                else if (text->font_size < 8)
                    text->font_size = 8;
                else if (text->font_size > 12)
                    text->font_size = 12;
                // max_width_follow_text 优先级低于 max-width
                std::string str_width_follow_text = CTinyXml2Helper::ElementAttribute(xml_node, "width_follow_text");
                if (str_width_follow_text == "true")
                    text->width_follow_text = true;
                else if (str_width_follow_text == "false")
                    text->width_follow_text = false;
            }
        }
        //专辑封面
        else if (item_name == "albumCover")
        {
            UiElement::AlbumCover* album_cover = dynamic_cast<UiElement::AlbumCover*>(element.get());
            if (album_cover != nullptr)
            {
                std::string str_square = CTinyXml2Helper::ElementAttribute(xml_node, "square");
                album_cover->square = CTinyXml2Helper::StringToBool(str_square.c_str());
                std::string str_show_info = CTinyXml2Helper::ElementAttribute(xml_node, "show_info");
                album_cover->show_info = CTinyXml2Helper::StringToBool(str_show_info.c_str());
            }
        }
        //频谱分析
        else if (item_name == "spectrum")
        {
            UiElement::Spectrum* spectrum = dynamic_cast<UiElement::Spectrum*>(element.get());
            if (spectrum != nullptr)
            {
                std::string str_draw_reflex = CTinyXml2Helper::ElementAttribute(xml_node, "draw_reflex");
                spectrum->draw_reflex = CTinyXml2Helper::StringToBool(str_draw_reflex.c_str());
                std::string str_fixed_width = CTinyXml2Helper::ElementAttribute(xml_node, "fixed_width");
                spectrum->fixed_width = CTinyXml2Helper::StringToBool(str_fixed_width.c_str());
                std::string str_type = CTinyXml2Helper::ElementAttribute(xml_node, "type");
                if (str_type == "64col")
                    spectrum->type = CUIDrawer::SC_64;
                else if (str_type == "32col")
                    spectrum->type = CUIDrawer::SC_32;
                else if (str_type == "16col")
                    spectrum->type = CUIDrawer::SC_16;
                else if (str_type == "8col")
                    spectrum->type = CUIDrawer::SC_8;
                //alignment
                std::string str_alignment = CTinyXml2Helper::ElementAttribute(xml_node, "alignment");
                if (str_alignment == "left")
                    spectrum->align = Alignment::LEFT;
                else if (str_alignment == "right")
                    spectrum->align = Alignment::RIGHT;
                else if (str_alignment == "center")
                    spectrum->align = Alignment::CENTER;
            }
        }
        //工具条
        else if (item_name == "toolbar")
        {
            UiElement::Toolbar* toolbar = dynamic_cast<UiElement::Toolbar*>(element.get());
            if (toolbar != nullptr)
            {
                std::string str_show_translate_btn = CTinyXml2Helper::ElementAttribute(xml_node, "show_translate_btn");
                toolbar->show_translate_btn = CTinyXml2Helper::StringToBool(str_show_translate_btn.c_str());
            }
        }
        //进度条
        else if (item_name == "progressBar")
        {
            UiElement::ProgressBar* progress_bar = dynamic_cast<UiElement::ProgressBar*>(element.get());
            if (progress_bar != nullptr)
            {
                std::string str_show_play_time = CTinyXml2Helper::ElementAttribute(xml_node, "show_play_time");
                progress_bar->show_play_time = CTinyXml2Helper::StringToBool(str_show_play_time.c_str());
                std::string str_play_time_both_side = CTinyXml2Helper::ElementAttribute(xml_node, "play_time_both_side");
                progress_bar->play_time_both_side = CTinyXml2Helper::StringToBool(str_play_time_both_side.c_str());
            }
        }
        //音量
        else if (item_name == "volume")
        {
            UiElement::Volume* volume = dynamic_cast<UiElement::Volume*>(element.get());
            if (volume != nullptr)
            {
                std::string str_show_text = CTinyXml2Helper::ElementAttribute(xml_node, "show_text");
                volume->show_text = CTinyXml2Helper::StringToBool(str_show_text.c_str());
                std::string str_adj_btn_on_top = CTinyXml2Helper::ElementAttribute(xml_node, "adj_btn_on_top");
                volume->adj_btn_on_top = CTinyXml2Helper::StringToBool(str_adj_btn_on_top.c_str());
            }
        }
        else if (item_name == "stackElement")
        {
            m_stack_elements[current_build_ui_element].push_back(element);
            UiElement::StackElement* stack_element = dynamic_cast<UiElement::StackElement*>(element.get());
            if (stack_element != nullptr)
            {
                std::string str_click_to_switch = CTinyXml2Helper::ElementAttribute(xml_node, "ckick_to_switch");
                if (!str_click_to_switch.empty())
                    stack_element->ckick_to_switch = CTinyXml2Helper::StringToBool(str_click_to_switch.c_str());
                std::string str_show_indicator = CTinyXml2Helper::ElementAttribute(xml_node, "show_indicator");
                if (!str_show_indicator.empty())
                    stack_element->show_indicator = CTinyXml2Helper::StringToBool(str_show_indicator.c_str());
            }
        }

        //递归调用此函数创建子节点
        CTinyXml2Helper::IterateChildNode(xml_node, [&](tinyxml2::XMLElement* xml_child)
            {
                std::shared_ptr<UiElement::Element> ui_child = BuildUiElementFromXmlNode(xml_child);
                ui_child->pParent = element.get();
                element->childLst.push_back(ui_child);
            });
    }
    return element;
}

std::vector<std::shared_ptr<UiElement::Element>>& CUserUi::GetStackElements()
{
    return m_stack_elements[GetCurrentUiType().get()];
}

void CUserUi::LoadUi()
{
    m_stack_elements.clear();
    tinyxml2::XMLDocument xml_doc;
    CTinyXml2Helper::LoadXmlFile(xml_doc, m_xml_path.c_str());
    tinyxml2::XMLElement* root = xml_doc.RootElement();
    m_ui_name = CCommon::StrToUnicode(CTinyXml2Helper::ElementAttribute(root, "name"), CodeType::UTF8_NO_BOM);
    std::string ui_index = CTinyXml2Helper::ElementAttribute(root, "index");
    if (!ui_index.empty())
        m_index = atoi(ui_index.c_str());
    CTinyXml2Helper::IterateChildNode(root, [&](tinyxml2::XMLElement* xml_child)
        {
            std::string item_name = CTinyXml2Helper::ElementName(xml_child);
            if (item_name == "ui")
            {
                std::string str_type = CTinyXml2Helper::ElementAttribute(xml_child, "type");
                if (str_type == "big")
                    m_root_ui_big = BuildUiElementFromXmlNode(xml_child);
                else if (str_type == "narrow")
                    m_root_ui_narrow = BuildUiElementFromXmlNode(xml_child);
                else if (str_type == "small")
                    m_root_ui_small = BuildUiElementFromXmlNode(xml_child);
                else
                    m_root_default = BuildUiElementFromXmlNode(xml_child);
            }
        });
}

void CUserUi::SwitchStackElement()
{
    m_draw_data.lyric_rect.SetRectEmpty();
    auto& stack_elements{ GetStackElements() };
    if (!stack_elements.empty())
    {
        UiElement::StackElement* stack_element = dynamic_cast<UiElement::StackElement*>(stack_elements.front().get());
        if (stack_element != nullptr)
            stack_element->SwitchDisplay();
    }
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

void CUserUi::UniqueUiIndex(std::vector<std::shared_ptr<CUserUi>>& ui_list)
{
    for (auto& ui : ui_list)
    {
        if (ui != nullptr)
        {
            //遍历UI列表，获取当前UI的序号
            int ui_index = ui->GetUiIndex();
            //如果还有其他UI的序号和当前UI相同
            auto ui_matched_index = FindUiByIndex(ui_list, ui_index, ui);
            if (ui_matched_index != nullptr)
            {
                //将找到的UI的序号设置为最大序号加1
                ui_matched_index->SetIndex(GetMaxUiIndex(ui_list) + 1);
            }
        }
    }
}

std::shared_ptr<CUserUi> CUserUi::FindUiByIndex(const std::vector<std::shared_ptr<CUserUi>>& ui_list, int ui_index, std::shared_ptr<CUserUi> except)
{
    for (const auto& ui : ui_list)
    {
        if (ui != nullptr && ui != except && ui->GetUiIndex() == ui_index)
            return ui;
    }
    return nullptr;
}

int CUserUi::GetMaxUiIndex(const std::vector<std::shared_ptr<CUserUi>>& ui_list)
{
    int index_max{};
    for (const auto& ui : ui_list)
    {
        if (ui != nullptr && ui->GetUiIndex() > index_max)
            index_max = ui->GetUiIndex();
    }
    return index_max;
}
