    /*
    -----------------------------------------------------------------------------
    This source file is part of OGRE
        (Object-oriented Graphics Rendering Engine)
    For the latest info, see http://ogre.sourceforge.net/
    
    Copyright � 2000-2002 The OGRE Team
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
    
    #ifndef __GuiElement_H__
    #define __GuiElement_H__
    
    #include "OgrePrerequisites.h"
    #include "OgreString.h"
    #include "OgreRenderable.h"
    #include "OgreStringInterface.h"
    #include "OgreGuiElementCommands.h"
    
    #include "OgreActionTarget.h"
    #include "OgreMouseTarget.h"
    #include "OgreMouseMotionTarget.h"
	#include "OgreColourValue.h"
    
    namespace Ogre {
    
    
      /** Enum describing how the position / size of an element is to be recorded. 
      */
      enum GuiMetricsMode
      {
          /// 'left', 'top', 'height' and 'width' are parametrics from 0.0 to 1.0
          GMM_RELATIVE,
          /// Positions & sizes are in absolute pixels
          GMM_PIXELS
      };
  
      /** Enum describing where '0' is in relation to the parent in the horizontal dimension.
      @remarks Affects how 'left' is interpreted.
      */
      enum GuiHorizontalAlignment
      {
          GHA_LEFT,
          GHA_CENTER,
          GHA_RIGHT
      };
      /** Enum describing where '0' is in relation to the parent in the vertical dimension.
      @remarks Affects how 'top' is interpreted.
      */
      enum GuiVerticalAlignment
      {
          GVA_TOP,
          GVA_CENTER,
          GVA_BOTTOM
      };

    /** Abstract definition of a 2D element to be displayed in an Overlay.
    @remarks
        This class abstracts all the details of a 2D element which will appear in
        an overlay. In fact, not all GuiElement instances can be directly added to an
        Overlay, only those which are GuiContainer instances (a subclass of this class).
        GuiContainer objects can contain any GuiElement however. This is just to 
        enforce some level of grouping on widgets.
    @par
        GuiElements should be managed using GuiManager. This class is responsible for
        instantiating / deleting elements, and also for accepting new types of element
        from plugins etc.
    @par
        Note that positions / dimensions of 2D screen elements are expressed as parametric
        values (0.0 - 1.0) because this makes them resolution-independent. However, most
        screen resolutions have an aspect ratio of 1.3333:1 (width : height) so note that
        in physical pixels 0.5 is wider than it is tall, so a 0.5x0.5 panel will not be
        square on the screen (but it will take up exactly half the screen in both dimensions).
    @par
        Because this class is designed to be extensible, it subclasses from StringInterface
        so its parameters can be set in a generic way.
    */
    class _OgreExport GuiElement : public StringInterface, public Renderable, public MouseTarget, public MouseMotionTarget, public ActionTarget
    {
    public:

    protected:
        // Command object for setting / getting parameters
        static GuiElementCommands::CmdLeft msLeftCmd;
        static GuiElementCommands::CmdTop msTopCmd;
        static GuiElementCommands::CmdWidth msWidthCmd;
        static GuiElementCommands::CmdHeight msHeightCmd;
        static GuiElementCommands::CmdMaterial msMaterialCmd;
        static GuiElementCommands::CmdCaption msCaptionCmd;
        static GuiElementCommands::CmdMetricsMode msMetricsModeCmd;
        static GuiElementCommands::CmdHorizontalAlign msHorizontalAlignCmd;
        static GuiElementCommands::CmdVerticalAlign msVerticalAlignCmd;
        static GuiElementCommands::CmdVisible msVisibleCmd;


        String mName;
        bool mVisible;
		bool mCloneable;
        Real mLeft;
        Real mTop;
        Real mWidth;
        Real mHeight;
        String mMaterialName;
        Material* mpMaterial;
        String mCaption;
		ColourValue mColour;

        GuiMetricsMode mMetricsMode;
        GuiHorizontalAlignment mHorzAlign;
        GuiVerticalAlignment mVertAlign;

        // Pixel-mode positions, used in GMM_PIXELS mode.
        short mPixelTop;
        short mPixelLeft;
        short mPixelWidth;
        short mPixelHeight;

        // Parent pointer
        GuiContainer* mParent;
        // Overlay attached to
        Overlay* mOverlay;

        // Derived positions from parent
        Real mDerivedLeft;
        Real mDerivedTop;
        bool mDerivedOutOfDate;

        /// Falg indicating if the vertex positons need recalculating
        bool mGeomPositionsOutOfDate;

        // Zorder for when sending to render queue
        // Derived from parent
        ushort mZOrder;


		// is element enabled
		bool mEnabled;


        /** Internal method which is triggered when the positions of the element get updated,
            meaning the element should be rebuilding it's mesh positions. Abstract since
            subclasses must implement this.
        */
        virtual void updatePositionGeometry(void) = 0;

        /** Internal method for setting up the basic parameter definitions for a subclass. 
        @remarks
            Because StringInterface holds a dictionary of parameters per class, subclasses need to
            call this to ask the base class to add it's parameters to their dictionary as well.
            Can't do this in the constructor because that runs in a non-virtual context.
        @par
            The subclass must have called it's own createParamDictionary before calling this method.
        */
        virtual void addBaseParameters(void);

    public:
        /// Constructor: do not call direct, use GuiManager::createElement
        GuiElement(const String& name);
        virtual ~GuiElement();

        /** Initialise gui element */
        virtual void initialise(void) = 0;

        /** Gets the name of this overlay. */
        const String& getName(void) const;


        /** Shows this element if it was hidden. */
        virtual void show(void);

        /** Hides this element if it was visible. */
        virtual void hide(void);

        /** Returns whether or not the element is visible. */
        bool isVisible(void);

		bool isEnabled();
		virtual void setEnabled(bool b);


        /** Sets the dimensions of this element in relation to the screen (1.0 = screen width/height). */
        void setDimensions(Real width, Real height);

        /** Sets the position of the top-left corner of the element, relative to the screen size
            (1.0 = screen width / height) */
        void setPosition(Real left, Real top);

        /** Sets the width of this element in relation to the screen (where 1.0 = screen width) */
        void setWidth(Real width);
        /** Gets the width of this element in relation to the screen (where 1.0 = screen width) */
        Real getWidth(void) const;

        /** Sets the height of this element in relation to the screen (where 1.0 = screen height) */
        void setHeight(Real height);
        /** Gets the height of this element in relation to the screen (where 1.0 = screen height) */
        Real getHeight(void) const;

        /** Sets the left of this element in relation to the screen (where 0 = far left, 1.0 = far right) */
        void setLeft(Real left);
        /** Gets the left of this element in relation to the screen (where 0 = far left, 1.0 = far right)  */
        Real getLeft(void) const;

        /** Sets the top of this element in relation to the screen (where 0 = top, 1.0 = bottom) */
        void setTop(Real Top);
        /** Gets the top of this element in relation to the screen (where 0 = top, 1.0 = bottom)  */
        Real getTop(void) const;


        /** Gets the name of the material this element uses. */
        virtual const String& getMaterialName(void) const;

        /** Sets the name of the material this element will use. 
        @remarks
            Different elements will use different materials. One constant about them
            all though is that a Material used for a GuiElement must have it's depth
            checking set to 'off', which means it always gets rendered on top. OGRE
            will set this flag for you if necessary. What it does mean though is that 
            you should not use the same Material for rendering GuiElements as standard 
            scene objects. It's fine to use the same textures, just not the same
            Material.
        */
        virtual void setMaterialName(const String& matName);


        // --- Renderable Overrides ---
        /** See Renderable */
        Material* getMaterial(void) const;

        // NB getRenderOperation not implemented, still abstract here

        /** See Renderable */
        void getWorldTransforms(Matrix4* xform);

        /** See Renderable */
        bool useIdentityProjection(void);

        /** See Renderable */
        bool useIdentityView(void);

		/** Tell the object to recalculate */
		virtual void _positionsOutOfDate(void);

        /** Internal method to update the element based on transforms applied. */
        virtual void _update(void);

        /** Updates this elements transform based on it's parent. */
        virtual void _updateFromParent(void);

        /** Internal method for notifying the gui element of it's parent and ultimate overlay. */
        virtual void _notifyParent(GuiContainer* parent, Overlay* overlay);

        /** Gets the 'left' position as derived from own left and that of parents. */
        virtual Real _getDerivedLeft(void);

        /** Gets the 'top' position as derived from own left and that of parents. */
        virtual Real _getDerivedTop(void);

        /** Internal method to notify the element when Zorder of parent overlay
         has changed.
         @remarks
            Overlays have explicit Z orders. GuiElements do not, they inherit the 
            ZOrder of the overlay, and the Zorder is incremented for every container
            nested within this to ensure that containers are displayed behind contained
            items. This method is used internally to notify the element of a change in
            final zorder which is used to render the element.
        */
        virtual void _notifyZOrder(ushort newZOrder);

        /** Internal method to put the contents onto the render queue. */
        virtual void _updateRenderQueue(RenderQueue* queue);

        /** Gets the type name of the element. All concrete subclasses must implement this. */
        virtual const String& getTypeName(void) = 0;

        /** Sets the caption on elements that support it. 
        @remarks
            This property doesn't do something on all elements, just those that support it.
            However, being a common requirement it is in the top-level interface to avoid
            having to set it via the StringInterface all the time.
        */
        virtual void setCaption(const String& text);

        /** Gets the caption for this element. */
        virtual const String& getCaption(void) const;

        /** Sets the colour on elements that support it. 
        @remarks
            This property doesn't do something on all elements, just those that support it.
            However, being a common requirement it is in the top-level interface to avoid
            having to set it via the StringInterface all the time.
        */
        virtual void setColour(const ColourValue& col);

        /** Gets the colour for this element. */
        virtual ColourValue getColour(void) const;

        /** Tells this element how to interpret the position and dimension values it is given.
        @remarks
            By default, GuiElements are positioned and sized according to relative dimensions
            of the screen. This is to ensure portability between different resolutions when you
            want things to be positioned and sized the same way across all resolutions. However, 
            sometimes you want things to be sized according to fixed pixels. In order to do this,
            you can call this method with the parameter GMM_PIXELS. Note that if you then want
            to place your element relative to the center, right or bottom of it's parent, you will
            need to use the setHorizontalAlignment and setVerticalAlignment methods.
        */
        virtual void setMetricsMode(GuiMetricsMode gmm);
        /** Retrieves the current settings of how the element metrics are interpreted. */
        virtual GuiMetricsMode getMetricsMode(void);
        /** Sets the horizontal origin for this element.
        @remarks
            By default, the horizontal origin for a GuiElement is the left edge of the parent container
            (or the screen if this is a root element). You can alter this by calling this method, which is
            especially useful when you want to use pixel-based metrics (see setMetricsMode) since in this
            mode you can't use relative positioning.
        @par
            For example, if you were using GMM_PIXELS metrics mode, and you wanted to place a 30x30 pixel
            crosshair in the center of the screen, you would use GHA_CENTER with a 'left' property of -15.
        @par
            Note that neither GHA_CENTER or GHA_RIGHT alter the position of the element based
            on it's width, you have to alter the 'left' to a negative number to do that; all this
            does is establish the origin. This is because this way you can align multiple things
            in the center and right with different 'left' offsets for maximum flexibility.
        */
        virtual void setHorizontalAlignment(GuiHorizontalAlignment gha);
        /** Gets the horizontal alignment for this element. */
        virtual GuiHorizontalAlignment getHorizontalAlignment(void);
        /** Sets the vertical origin for this element. 
        @remarks
            By default, the vertical origin for a GuiElement is the top edge of the parent container
            (or the screen if this is a root element). You can alter this by calling this method, which is
            especially useful when you want to use pixel-based metrics (see setMetricsMode) since in this
            mode you can't use relative positioning.
          @par
              For example, if you were using GMM_PIXELS metrics mode, and you wanted to place a 30x30 pixel
              crosshair in the center of the screen, you would use GHA_CENTER with a 'top' property of -15.
          @par
              Note that neither GVA_CENTER or GVA_BOTTOM alter the position of the element based
              on it's height, you have to alter the 'top' to a negative number to do that; all this
              does is establish the origin. This is because this way you can align multiple things
              in the center and bottom with different 'top' offsets for maximum flexibility.
          */
          virtual void setVerticalAlignment(GuiVerticalAlignment gva);
          /** Gets the vertical alignment for this element. */
          virtual GuiVerticalAlignment getVerticalAlignment(void);
  
  
  
    
    		/** Returns true if xy is within the constraints of the component */
    		virtual bool contains(Real x, Real y) const;
    
    		/** Returns true if xy is within the constraints of the component */
    		virtual GuiElement* findElementAt(Real x, Real y);		// relative to parent
    
    		/**
    		 * Processes events occurring on this component. By default this
    		 * method calls the appropriate process event method
    		 */
    		virtual void processEvent(InputEvent* e);
    
    		/**
    		 * returns false as this class is not a container type 
    		 */
    		inline virtual bool isContainer()
    		{ return false; }
    
  		inline virtual bool isKeyEnabled()
  		{ return false; }
  
    		inline virtual bool isCloneable()
    		{ return mCloneable; }
    
    		inline virtual void setCloneable(bool c)
    		{ mCloneable = c; }
    	
    		/**
    		 * Returns the parent container.
    		 */
    		PositionTarget* getPositionTargetParent() ;
    
    		/**
  		 * Returns the parent container.
  		 */
  		GuiContainer* getParent() ;
  
  		/**
    		 * Returns the zOrder of the element
    		 */
    		inline ushort getZOrder() const
    		{ return mZOrder; }
    
            /** Overridden from Renderable */
            Real getSquaredViewDepth(const Camera* cam) const 
            { 
                return 10000 - getZOrder(); 
            }
    
    
    	    void copyFromTemplate(GuiElement* templateGui);
  
    	};
    
    
    
    }
    
    
    #endif
    
