/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2002 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/

#include "OgreInput.h"
#include "OgreCursor.h"
#include "OgreEventQueue.h"
#include "OgreKeyEvent.h"


namespace Ogre {
	static KeyChars sKeyChars;
	bool InputReader::sKeysInitialised = false;
    //-----------------------------------------------------------------------
    InputReader::InputReader()
    {
		mCursor = 0;
		mEventQueue = 0;
		mUseBufferedKeys = false;
		mUseBufferedMouse = false;
		if (!sKeysInitialised)
		{
			setupKeyChars();
			sKeysInitialised = true;
		}
    }

    //-----------------------------------------------------------------------
    void InputReader::useBufferedInput(EventQueue* pEventQueue, bool keys, bool mouse) 
    {
		mEventQueue = pEventQueue;

		if (mCursor)
			delete mCursor;

		mCursor = new Cursor();

		// initial states of buffered don't call setBufferedInput 
		// because that can be overriden (in the future) to save releasing and acquiring unchanged inputs
		// if we ever decide to release and acquire devices
		mUseBufferedKeys = keys;
		mUseBufferedMouse = mouse;
    }

    //-----------------------------------------------------------------------
    void InputReader::setBufferedInput(bool keys, bool mouse) 
    {
		mUseBufferedKeys = keys;
		mUseBufferedMouse = mouse;
    }


    //-----------------------------------------------------------------------
    InputReader::~InputReader()
    {
		if (mCursor)
		{
			delete mCursor;
		}
    }

    //-----------------------------------------------------------------------
	void InputReader::triggerMouseButton(int nMouseCode, bool mousePressed)
	{
		if (mousePressed)
		{
			mModifiers |= nMouseCode;
			createMouseEvent(MouseEvent::ME_MOUSE_PRESSED, nMouseCode);

		}
		else
		{	// button up... trigger MouseReleased, and MouseClicked
			mModifiers &= !nMouseCode;
			createMouseEvent(MouseEvent::ME_MOUSE_RELEASED, nMouseCode);
			//createMouseEvent(MouseEvent::ME_MOUSE_CLICKED, nMouseCode);	JCA - moved to EventDispatcher
		}
	}

    //-----------------------------------------------------------------------
	void InputReader::createMouseEvent(int id, int button)
	{
		MouseEvent* me =
            new MouseEvent(
                NULL, id, button, 0, // hack fix time
			    mModifiers,
                mCursor->getX(), mCursor->getY(), mCursor->getZ(),
                mCursor->getRelX(), mCursor->getRelY(), mCursor->getRelZ(),
                0
            );	// hack fix click count

		mCursor->processEvent(me);
		mEventQueue->push(me);

	}



    //-----------------------------------------------------------------------
	void InputReader::createKeyEvent(int id, int key)
	{
		KeyEvent* ke = new KeyEvent(NULL, id, key, 0, // hack fix time
			mModifiers);	// hack fix click count
		mEventQueue->push(ke);

	}
 
    //-----------------------------------------------------------------------
	void InputReader::mouseMoved()
	{

		if (mModifiers & InputEvent::BUTTON_ANY_MASK)	// don't need to know which button. you can get that from the modifiers
		{
			createMouseEvent(MouseEvent::ME_MOUSE_DRAGGED, 0);
		}
		else
		{
			createMouseEvent(MouseEvent::ME_MOUSE_MOVED, 0);
		}

	}
    //-----------------------------------------------------------------------
	void InputReader::addCursorMoveListener(MouseMotionListener* c)
	{
		mCursor->addMouseMotionListener(c);
	}
    //-----------------------------------------------------------------------
	void InputReader::removeCursorMoveListener(MouseMotionListener* c)
	{
		mCursor->removeMouseMotionListener(c);
	}
    //-----------------------------------------------------------------------

	void InputReader::keyChanged(int key, bool down)
	{

		if (down)
		{
			createKeyEvent(KeyEvent::KE_KEY_PRESSED, key);
		}
		else
		{
			createKeyEvent(KeyEvent::KE_KEY_RELEASED, key);
			createKeyEvent(KeyEvent::KE_KEY_CLICKED, key);
		}
	}


	char InputReader::getKeyChar(int keyCode)
	{
		char c = 0;
        KeyChars::iterator i = sKeyChars.find(keyCode);
        if (i != sKeyChars.end())
            c = i->second;

		return c;

	}

	void InputReader::setupKeyChars()
	{

        sKeyChars.insert( KeyChars::value_type( KC_1			   ,'1') );
        sKeyChars.insert( KeyChars::value_type( KC_2               ,'2') );
        sKeyChars.insert( KeyChars::value_type( KC_3               ,'3') );
        sKeyChars.insert( KeyChars::value_type( KC_4               ,'4') );
        sKeyChars.insert( KeyChars::value_type( KC_5               ,'5') );
        sKeyChars.insert( KeyChars::value_type( KC_6               ,'6') );
        sKeyChars.insert( KeyChars::value_type( KC_7               ,'7') );
        sKeyChars.insert( KeyChars::value_type( KC_8               ,'8') );
        sKeyChars.insert( KeyChars::value_type( KC_9               ,'9') );
        sKeyChars.insert( KeyChars::value_type( KC_0               ,'0') );
        sKeyChars.insert( KeyChars::value_type( KC_MINUS           ,'-') );/* - on main keyboard */
        sKeyChars.insert( KeyChars::value_type( KC_EQUALS          ,'=') );
        sKeyChars.insert( KeyChars::value_type( KC_Q               ,'q') );
        sKeyChars.insert( KeyChars::value_type( KC_W               ,'w') );
        sKeyChars.insert( KeyChars::value_type( KC_E               ,'e') );
        sKeyChars.insert( KeyChars::value_type( KC_R               ,'r') );
        sKeyChars.insert( KeyChars::value_type( KC_T               ,'t') );
        sKeyChars.insert( KeyChars::value_type( KC_Y               ,'y') );
        sKeyChars.insert( KeyChars::value_type( KC_U               ,'u') );
        sKeyChars.insert( KeyChars::value_type( KC_I               ,'i') );
        sKeyChars.insert( KeyChars::value_type( KC_O               ,'o') );
        sKeyChars.insert( KeyChars::value_type( KC_P               ,'p') );
        sKeyChars.insert( KeyChars::value_type( KC_LBRACKET        ,'[') );
        sKeyChars.insert( KeyChars::value_type( KC_RBRACKET        ,']') );
        sKeyChars.insert( KeyChars::value_type( KC_A               ,'a') );
        sKeyChars.insert( KeyChars::value_type( KC_S               ,'s') );
        sKeyChars.insert( KeyChars::value_type( KC_D               ,'d') );
        sKeyChars.insert( KeyChars::value_type( KC_F               ,'f') );
        sKeyChars.insert( KeyChars::value_type( KC_G               ,'g') );
        sKeyChars.insert( KeyChars::value_type( KC_H               ,'h') );
        sKeyChars.insert( KeyChars::value_type( KC_J               ,'j') );
        sKeyChars.insert( KeyChars::value_type( KC_K               ,'k') );
        sKeyChars.insert( KeyChars::value_type( KC_L               ,'l') );
        sKeyChars.insert( KeyChars::value_type( KC_SEMICOLON       ,';') );
        sKeyChars.insert( KeyChars::value_type( KC_APOSTROPHE      ,'\'') );
        sKeyChars.insert( KeyChars::value_type( KC_GRAVE           ,'`') );  /* accent grave */
        sKeyChars.insert( KeyChars::value_type( KC_BACKSLASH       ,'\\') );
        sKeyChars.insert( KeyChars::value_type( KC_Z               ,'z') );
        sKeyChars.insert( KeyChars::value_type( KC_X               ,'x') );
        sKeyChars.insert( KeyChars::value_type( KC_C               ,'c') );
        sKeyChars.insert( KeyChars::value_type( KC_V               ,'v') );
        sKeyChars.insert( KeyChars::value_type( KC_B               ,'b') );
        sKeyChars.insert( KeyChars::value_type( KC_N               ,'n') );
        sKeyChars.insert( KeyChars::value_type( KC_M               ,'m') );
        sKeyChars.insert( KeyChars::value_type( KC_COMMA           ,',') );
        sKeyChars.insert( KeyChars::value_type( KC_PERIOD          ,'.') );  /* . on main keyboard */
        sKeyChars.insert( KeyChars::value_type( KC_SLASH           ,'/') );  /* '/' on main keyboard */
        sKeyChars.insert( KeyChars::value_type( KC_MULTIPLY        ,'*') );  /* * on numeric keypad */
        sKeyChars.insert( KeyChars::value_type( KC_SPACE           ,' ') );
        sKeyChars.insert( KeyChars::value_type( KC_NUMPAD7         ,'7') );
        sKeyChars.insert( KeyChars::value_type( KC_NUMPAD8         ,'8') );
        sKeyChars.insert( KeyChars::value_type( KC_NUMPAD9         ,'9') );
        sKeyChars.insert( KeyChars::value_type( KC_SUBTRACT        ,'-') );  /* - on numeric keypad */
        sKeyChars.insert( KeyChars::value_type( KC_NUMPAD4         ,'4') );
        sKeyChars.insert( KeyChars::value_type( KC_NUMPAD5         ,'5') );
        sKeyChars.insert( KeyChars::value_type( KC_NUMPAD6         ,'6') );
        sKeyChars.insert( KeyChars::value_type( KC_ADD             ,'+') );  /* + on numeric keypad */
        sKeyChars.insert( KeyChars::value_type( KC_NUMPAD1         ,'1') );
        sKeyChars.insert( KeyChars::value_type( KC_NUMPAD2         ,'2') );
        sKeyChars.insert( KeyChars::value_type( KC_NUMPAD3         ,'3') );
        sKeyChars.insert( KeyChars::value_type( KC_NUMPAD0         ,'0') );
        sKeyChars.insert( KeyChars::value_type( KC_DECIMAL         ,'.') );  /* . on numeric keypad */
        sKeyChars.insert( KeyChars::value_type( KC_NUMPADEQUALS    ,'=') );  /* = on numeric keypad (NEC PC98) */
        sKeyChars.insert( KeyChars::value_type( KC_AT              ,'@') );  /*                     (NEC PC98) */
        sKeyChars.insert( KeyChars::value_type( KC_COLON           ,':') );  /*                     (NEC PC98) */
        sKeyChars.insert( KeyChars::value_type( KC_UNDERLINE       ,'_') );  /*                     (NEC PC98) */
        sKeyChars.insert( KeyChars::value_type( KC_NUMPADCOMMA     ,',') );  /* , on numeric keypad (NEC PC98) */
        sKeyChars.insert( KeyChars::value_type( KC_DIVIDE          ,'/') );  /* / on numeric keypad */
	}
}
