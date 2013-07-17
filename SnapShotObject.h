#ifndef _SNAPSHOTOBJECT_H_
#define _SNAPSHOTOBJECT_H_

#include <btBulletDynamicsCommon.h>
#include <OgreVector3.h>
#include <OgreQuaternion.h>

namespace LC
{
	struct SnapShotObject
	{
		SnapShotObject() {};

		SnapShotObject(std::string name, const btVector3 &pos,
			const btQuaternion &orient, const btVector3 &vel)
		{
			SceneNodeName = name;
			Position[0] = pos.x();
			Position[1] = pos.y();
			Position[2] = pos.z();
			Orientation[0] = orient.w();
			Orientation[1] = orient.x();
			Orientation[2] = orient.y();
			Orientation[3] = orient.z();
			Velocity[0] = vel.x();
			Velocity[1] = vel.y();
			Velocity[2] = vel.z();
		};

		std::string SceneNodeName;
		float Position[3]; // Vector: X, Y, Z
		float Orientation[4]; // Quaternion: W, X, Y, Z
		float Velocity[3]; // Vector: X, Y, Z

		template<class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & SceneNodeName;
			ar & Position;
			ar & Orientation;
			ar & Velocity;
		};

		Ogre::Vector3 getPosition()
		{
			return Ogre::Vector3(Position[0], Position[1], Position[2]);
		};

		Ogre::Quaternion getOrientation()
		{
			return Ogre::Quaternion(Orientation[0], Orientation[1],
				Orientation[2], Orientation[3]);
		};

		Ogre::Vector3 getVelocity()
		{
			return Ogre::Vector3(Velocity[0], Velocity[1], Velocity[2]);
		};
		std::string getName()
		{
			return SceneNodeName;
		};
	};
}

#endif