#include <btBulletCollisionCommon.h>

#include "GameObject.h"


// From http://bulletphysics.org/mediawiki-1.5.8/index.php/Collision_Callbacks_and_Triggers
struct ContactSensorCallback : public btCollisionWorld::ContactResultCallback {
	
	//! Constructor, pass whatever context you want to have available when processing contacts
	/*! You may also want to set m_collisionFilterGroup and m_collisionFilterMask
	 *  (supplied by the superclass) for needsCollision() */
	ContactSensorCallback(btRigidBody& tgtBody)
		: btCollisionWorld::ContactResultCallback(), body(tgtBody) { }
	
	btRigidBody& body; //!< The body the sensor is monitoring
	//YourContext& ctxt; //!< External information for contact processing
	
	//! If you don't want to consider collisions where the bodies are joined by a constraint, override needsCollision:
	/*! However, if you use a btCollisionObject for #body instead of a btRigidBody,
	 *  then this is unnecessary—checkCollideWithOverride isn't available */
	virtual bool needsCollision(btBroadphaseProxy* proxy) const {
		// superclass will check m_collisionFilterGroup and m_collisionFilterMask
		if(!btCollisionWorld::ContactResultCallback::needsCollision(proxy))
			return false;
		// if passed filters, may also want to avoid contacts between constraints
		return body.checkCollideWithOverride(static_cast<btCollisionObject*>(proxy->m_clientObject));
	}
	
	//! Called with each contact for your own processing (e.g. test if contacts fall in within sensor parameters)
	virtual btScalar addSingleResult(btManifoldPoint& cp,
		const btCollisionObject* colObj0,int partId0,int index0,
		const btCollisionObject* colObj1,int partId1,int index1)
	{
		LC::GameObject* obA_GO = ((LC::GameObject*)colObj0->getUserPointer());
		LC::GameObject* obB_GO = ((LC::GameObject*)colObj0->getUserPointer());

		std::string obA_name = obA_GO->m_Name;
		std::string obB_name = obB_GO->m_Name;
		if(obA_name.substr(0,4) == "Play")
			dynamic_cast<LC::ServerPlayer*>(obA_GO)->m_IsDead = true;
		else if(obB_name.substr(0,4) == "Play")
			dynamic_cast<LC::ServerPlayer*>(obB_GO)->m_IsDead = true;
		
		// do stuff with the collision point
		return 0; // not actually sure if return value is used for anything...?
	}
};