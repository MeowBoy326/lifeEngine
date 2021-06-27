/**
 * @file
 * @addtogroup Engine Engine
 *
 * Copyright BSOD-Games, All Rights Reserved.
 * Authors: Yehor Pohuliaka (zombiHello)
 */

#ifndef BASEDEVICECONTEXTRHI_H
#define BASEDEVICECONTEXTRHI_H

/**
 * @ingroup Engine
 * @brief Base class for work with device context RHI
 */
class FBaseDeviceContextRHI
{
public:
	/**
	 * @brief Destructor
	 */
	virtual						~FBaseDeviceContextRHI() {}

	/**
	 * @brief Clear surface
	 * 
	 * @param[in] InSurface Surface for rendering
	 * @param[in] InColor Color for clearing render target
	 */
	virtual void				ClearSurface( class FBaseSurfaceRHI* InSurface, const class FColor& InColor ) {}
};

#endif // !BASEDEVICECONTEXTRHI_H