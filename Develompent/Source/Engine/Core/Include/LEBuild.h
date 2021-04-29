/**
 * @file
 * @addtogroup Core Core
 *
 * Copyright BSOD-Games, All Rights Reserved.
 * Authors: Yehor Pohuliaka (zombiHello)
 */

#ifndef LEBUILD_H
#define LEBUILD_H

#ifndef SHIPPING_BUILD
	#define SHIPPING_BUILD			SHIPPING
#endif // !SHIPPING_BUILD

#ifndef DO_CHECK
	#define DO_CHECK				!SHIPPING_BUILD
#endif // !DO_CHECK

#ifndef WITH_EDITOR
	#define  WITH_EDITOR			!SHIPPING_BUILD
#endif // !WITH_EDITOR

#endif // !LEBUILD_H