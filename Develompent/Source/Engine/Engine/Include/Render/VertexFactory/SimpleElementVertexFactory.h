/**
 * @file
 * @addtogroup Engine Engine
 *
 * Copyright Broken Singularity, All Rights Reserved.
 * Authors: Yehor Pohuliaka (zombiHello)
 */

#ifndef SIMPLEELEMENTVERTEXFACTORY_H
#define SIMPLEELEMENTVERTEXFACTORY_H

#include "Math/Math.h"
#include "Math/Color.h"
#include "Render/VertexFactory/VertexFactory.h"
#include "Render/RenderUtils.h"

 /**
  * @ingroup Engine
  * Simple element vertex type
  */
struct FSimpleElementVertexType
{
	FVector4D		position;		/**< Position vertex */
	FVector2D		texCoord;		/**< Texture coords */
	FColor			color;			/**< Color */

	/**
	 * Overload operator ==
	 */
	bool FORCEINLINE operator==( const FSimpleElementVertexType& InOther ) const
	{
		return position == InOther.position && texCoord == InOther.texCoord && color == InOther.color;
	}
};

/**
 * @ingroup Engine
 * The simple element vertex declaration resource type
 */
class FSimpleElementVertexDeclaration : public FRenderResource
{
public:
	/**
	 * @brief Initializes the RHI resources used by this resource.
	 * Called when the resource is initialized.
	 * This is only called by the rendering thread.
	 */
	virtual void InitRHI() override;

	/**
	 * @brief Releases the RHI resources used by this resource.
	 * Called when the resource is released.
	 * This is only called by the rendering thread.
	 */
	virtual void ReleaseRHI() override;

	/**
	 * @brief Get vertex declaration RHI
	 * @return Return vertex declaration RHI
	 */
	FORCEINLINE FVertexDeclarationRHIRef GetVertexDeclarationRHI()
	{
		return vertexDeclarationRHI;
	}

private:
	FVertexDeclarationRHIRef		vertexDeclarationRHI;		/**< Vertex declaration RHI */
};

/**
 * @ingroup Engine
 * Global resource of simple element vertex declaration
 */
extern TGlobalResource< FSimpleElementVertexDeclaration >			GSimpleElementVertexDeclaration;

/**
 * @ingroup Engine
 * Simple element vertex factory
 */
class FSimpleElementVertexFactory : public FVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE( FSimpleElementVertexFactory )

public:
	enum EStreamSourceSlot
	{
		SSS_Main			= 0		/**< Main vertex buffer */
	};

	/**
	 * @brief Initializes the RHI resources used by this resource.
	 * Called when the resource is initialized.
	 * This is only called by the rendering thread.
	 */
	virtual void InitRHI() override;

	/**
	 * @brief Get type hash
	 * @return Return hash of vertex factory
	 */
	virtual uint64 GetTypeHash() const override;

	/**
	 * @brief Construct vertex factory shader parameters
	 *
	 * @param InShaderFrequency Shader frequency
	 * @return Return instance of vertex factory shader parameters
	 */
	static FVertexFactoryShaderParameters* ConstructShaderParameters( EShaderFrequency InShaderFrequency );
};

#endif // !SIMPLEELEMENTVERTEXFACTORY_H
