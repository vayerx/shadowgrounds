#ifndef INCLUDED_FROZENBYTE_D6_JOINT_H
#define INCLUDED_FROZENBYTE_D6_JOINT_H

#include "joint_base.h"

class NxScene;
class NxD6JointDesc;

namespace frozenbyte {
    namespace physics {
        class JointDeformingInfo;
        class JointDeformingListener;

        class D6Joint : public JointBase {
        private:
            struct DeformData;
            boost::shared_ptr<DeformData> deformData;
            boost::shared_ptr<NxD6JointDesc> jointDesc;

        public:
            D6Joint(NxScene &scene, const NxD6JointDesc &desc, boost::shared_ptr<ActorBase> &a,
                    boost::shared_ptr<ActorBase> &b);
            ~D6Joint();

            void reconnect(boost::shared_ptr<ActorBase> actor1, boost::shared_ptr<ActorBase> actor2);

            // deforms joint when point<->joint angle limit is exceeded
            void setJointDeforming(const JointDeformingInfo *info);
            void setJointDeformingListener(JointDeformingListener *listener);
            void handleDeforming(NxScene &scene);

            bool isValid() const;
        };

    } // physics
}     // frozenbyte

#endif
