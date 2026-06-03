#pragma once
#include <JuceHeader.h>

using namespace juce ;

class CustomLookAndFeel : public juce::LookAndFeel_V4{
    
public:
    
    CustomLookAndFeel()
    {
        
        //Colour Scheme
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colours::black);
        setColour(juce::Label::textColourId, juce::Colours::white);
        setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        setColour(juce::GroupComponent::outlineColourId, juce::Colours::white);
        setColour(juce::GroupComponent::textColourId, juce::Colours::white);
        
        setColour(juce::PopupMenu::backgroundColourId, juce::Colours::black);
        setColour(juce::PopupMenu::textColourId, juce::Colours::white);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colours::grey);
        setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::black);
        
        setColour(juce::ComboBox::backgroundColourId, juce::Colours::black);
        setColour(juce::ComboBox::outlineColourId, juce::Colours::white);
        setColour(juce::ComboBox::textColourId, juce::Colours::white);
        setColour(juce::ComboBox::arrowColourId, juce::Colours::white);
        
    }
    
    
    juce::Font getLabelFont(juce::Label&) override { return bitcount.withHeight(14.0f); }
    juce::Font getComboBoxFont(juce::ComboBox&) override { return bitcount.withHeight(14.0f); }
    juce::Font getTextButtonFont(juce::TextButton&, int) override { return bitcount.withHeight(14.0f); }
    
    void drawGroupComponentOutline (juce::Graphics& g, int width, int height, const juce::String& text, const juce::Justification& position, juce::GroupComponent& group) override{
        
        const float textH = 15.0f;
        const float indent = 3.0f;
        const float textEdgeGap = 4.0f;
        auto cs = 5.0f;

        Font f = bitcount.withHeight(textH);

        Path p;
        auto x = indent;
        auto y = f.getAscent() - 3.0f;
        auto w = jmax (0.0f, (float) width - x * 2.0f);
        auto h = jmax (0.0f, (float) height - y  - indent);
        cs = jmin (cs, w * 0.5f, h * 0.5f);
        auto cs2 = 2.0f * cs;

        auto textW = text.isEmpty() ? 0
                                    : jlimit (0.0f,
                                              jmax (0.0f, w - cs2 - textEdgeGap * 2),
                                              (float) GlyphArrangement::getStringWidthInt (f, text) + textEdgeGap * 2.0f);
        auto textX = cs + textEdgeGap;

        if (position.testFlags (Justification::horizontallyCentred))
            textX = cs + (w - cs2 - textW) * 0.5f;
        else if (position.testFlags (Justification::right))
            textX = w - cs - textW - textEdgeGap;

        p.startNewSubPath (x + textX + textW, y);
        p.lineTo (x + w - cs, y);

        p.addArc (x + w - cs2, y, cs2, cs2, 0, MathConstants<float>::halfPi);
        p.lineTo (x + w, y + h - cs);

        p.addArc (x + w - cs2, y + h - cs2, cs2, cs2, MathConstants<float>::halfPi, MathConstants<float>::pi);
        p.lineTo (x + cs, y + h);

        p.addArc (x, y + h - cs2, cs2, cs2, MathConstants<float>::pi, MathConstants<float>::pi * 1.5f);
        p.lineTo (x, y + cs);

        p.addArc (x, y, cs2, cs2, MathConstants<float>::pi * 1.5f, MathConstants<float>::twoPi);
        p.lineTo (x + textX, y);

        auto alpha = group.isEnabled() ? 1.0f : 0.5f;

        g.setColour (group.findColour (GroupComponent::outlineColourId)
                        .withMultipliedAlpha (alpha));

        g.strokePath (p, PathStrokeType (2.0f));

        g.setColour (group.findColour (GroupComponent::textColourId)
                        .withMultipliedAlpha (alpha));
        g.setFont (f);
        g.drawText (text,
                    roundToInt (x + textX), 0,
                    roundToInt (textW),
                    roundToInt (textH),
                    Justification::centred, true);
    }
    
    juce::Font getPopupMenuFont() override
    {
        return bitcount.withHeight(18.0f);
    }




    
private:
    
    juce::Font bitcount{ juce::FontOptions(juce::Typeface::createSystemTypefaceFor(BinaryData::BitcountPropSingle_ttf , BinaryData::BitcountPropSingle_ttfSize))};
};
