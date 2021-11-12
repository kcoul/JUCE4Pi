/*
  ==============================================================================

    stm_ParameterAttachment.h
    Created: 28 Apr 2020 1:20:47pm
    Author:  Spenser Saling
    Note: This code uses the structure of AudioProcessorValueTreeState::ButtonAttachment found in juce_AudioProcessorValueTreeState.cpp
 
  ==============================================================================
*/

#pragma once

using namespace juce;

namespace stm {

struct ParameterAttachment : public AudioProcessorValueTreeState::Listener,
                            public AsyncUpdater
{
    ParameterAttachment (AudioProcessorValueTreeState& s, const String& p)
    : state(s), paramID(p), lastValue(0)
    {
        state.addParameterListener(paramID, this);
        sendInitialUpdate();
    }
    
    ~ParameterAttachment () override
    {
        state.removeParameterListener (paramID, this);
    }
    
    /**
     Call to safely set the parameter value
     Ensures thread safety. Notifys the host. Utilizes undo manager if available.
     */
    void setValue (float newValue) {
        const ScopedLock selfCallbackLock (selfCallbackMutex);

        if (! ignoreCallbacks)
        {
            beginParameterChange();
            setNewDenormalisedValue (newValue);
            endParameterChange();
        }
    }
    
    void parameterChanged (const String&, float newValue) override
    {
        lastValue = newValue;

        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            cancelPendingUpdate();
            setValueExternal (newValue);
        }
        else
        {
            triggerAsyncUpdate();
        }
    }
    
    void handleAsyncUpdate() override
    {
        setValueExternal (lastValue);
    }
    
    struct Listener
    {
        virtual ~Listener(){}
        /**
         Called when a value is changed internally (e.g. from the host)
         Use this to callback to update gui elements
         */
        virtual void valueUpdated(ParameterAttachment* attachment, float newValue) = 0;
    };
    
    void setListener(Listener* newListener){ listener = newListener; }
    void removeListener() { listener = nullptr; }
    
    float getValue(){
        return lastValue;
    }
    
private:
    AudioProcessorValueTreeState& state;
    String paramID;
    float lastValue;
    Listener* listener = nullptr;
    bool ignoreCallbacks;
    CriticalSection selfCallbackMutex;
    
    //Called when a parameterChange is registered (e.g. the parameter was updated by the host)
    void setValueExternal (float newValue)
    {
        const ScopedLock selfCallbackLock (selfCallbackMutex);
        {
            ScopedValueSetter<bool> svs (ignoreCallbacks, true);
            if (listener != nullptr) {
                listener -> valueUpdated(this, newValue);
            }
            setValue(newValue);
        }
    }
    
    void sendInitialUpdate()
    {
        if (auto* v = state.getRawParameterValue (paramID))
            parameterChanged (paramID, *v);
    }
    
    void beginParameterChange()
    {
        if (auto* p = state.getParameter (paramID))
        {
            if (state.undoManager != nullptr)
                state.undoManager->beginNewTransaction();

            p->beginChangeGesture();
        }
    }
    
    void setNewDenormalisedValue (float newDenormalisedValue)
    {
        if (auto* p = state.getParameter (paramID))
        {
            const float newValue = state.getParameterRange (paramID)
            .convertTo0to1 (newDenormalisedValue);
            
            if (p->getValue() != newValue)
                p->setValueNotifyingHost (newValue);
        }
    }

    void endParameterChange()
    {
        if (AudioProcessorParameter* p = state.getParameter (paramID))
            p->endChangeGesture();
    }


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterAttachment)
};


} //namespace stm
