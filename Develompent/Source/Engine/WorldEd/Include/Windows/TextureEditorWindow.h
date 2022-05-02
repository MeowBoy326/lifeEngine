/**
 * @file
 * @addtogroup WorldEd World editor
 *
 * Copyright Broken Singularity, All Rights Reserved.
 * Authors: Yehor Pohuliaka (zombiHello)
 */

#ifndef TEXTUREEDITORWINDOW_H
#define TEXTUREEDITORWINDOW_H

#include <QWidget>
#include "Render/Texture.h"

namespace Ui
{
	class WeTextureEditorWindow;
}

/**
 * @ingroup WorldEd
 * Class window of texture editor
 */
class WeTextureEditorWindow : public QWidget
{
	Q_OBJECT

public:
	/**
	 * Constructor
	 * 
	 * @param InTexture2D	Texture 2D to edit
	 * @param InParent		Parent widget
	 */
	WeTextureEditorWindow( FTexture2D* InTexture2D, QWidget* InParent = nullptr );

	/**
	 * Destructor
	 */
	virtual ~WeTextureEditorWindow();

private:
	/**
	 * Init UI
	 */
	void InitUI();

	Ui::WeTextureEditorWindow*		ui;			/**< Qt UI */
	FTexture2DRef					texture2D;	/**< Editable asset */
};

#endif // MAINWINDOW_H