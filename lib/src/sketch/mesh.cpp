/* This file is part of Dilay
 * Copyright © 2015 Alexander Bau
 * Use and redistribute under the terms of the GNU General Public License
 */
#include <glm/gtx/rotate_vector.hpp>
#include "../mesh.hpp"
#include "color.hpp"
#include "config.hpp"
#include "mesh-util.hpp"
#include "primitive/ray.hpp"
#include "primitive/sphere.hpp"
#include "render-mode.hpp"
#include "sketch/mesh.hpp"
#include "sketch/node-intersection.hpp"

namespace {
  struct RenderConfig {
    bool  renderWireframe;
    Color nodeColor;
    Color bubbleColor;

    RenderConfig ()
      : renderWireframe (false)
    {}
  };

  PrimSphere nodeSphere (const SketchNode& node) {
    return PrimSphere (node.data ().position (), node.data ().radius ());
  }
}

struct SketchMesh::Impl {
  SketchMesh*                   self;
  const unsigned int            index;
  std::unique_ptr <SketchNode> _root;
  Mesh                          nodeMesh;
  Mesh                          wireframeMesh;
  RenderConfig                  renderConfig;

  Impl (SketchMesh* s, unsigned int i)
    : self  (s)
    , index (i)
  {
    this->nodeMesh = MeshUtil::icosphere (3);
    this->nodeMesh.bufferData ();

    this->wireframeMesh = MeshUtil::cone (16);
    this->wireframeMesh.renderMode ().flatShading (true);
    this->wireframeMesh.position   (glm::vec3 (0.0f, 0.5f, 0.0f));
    this->wireframeMesh.normalize  ();
    this->wireframeMesh.bufferData ();
  }

  bool operator== (const SketchMesh& other) const {
    return this->index == other.index ();
  }

  bool operator!= (const SketchMesh& other) const {
    return ! this->operator== (other);
  }

  bool hasRoot () const {
    return bool (this->_root);
  }

  SketchNode& root () {
    assert (this->hasRoot ());
    return *this->_root;
  }

  const SketchNode& root () const {
    assert (this->hasRoot ());
    return *this->_root;
  }

  void fromTree (const SketchNode& tree) {
    assert (this->hasRoot () == false);
    this->_root = std::make_unique <SketchNode> (tree);
  }

  void reset () {
    this->_root.reset ();
  }

  bool intersects (const PrimRay& ray, SketchNodeIntersection& intersection) {
    if (this->hasRoot ()) {
      this->_root->forEachNode ([this, &ray, &intersection] (SketchNode& node) {
        float t;
        if (IntersectionUtil::intersects (ray, nodeSphere (node), &t)) {
          const glm::vec3 p = ray.pointAt (t);
          intersection.update ( t, p, glm::normalize (p - node.data ().position ())
                              , *this->self, node );
        }
      });
    }
    return intersection.isIntersection ();
  }

  void render (Camera& camera) {
    if (this->hasRoot ()) {
      this->_root->forEachConstNode ([this, &camera] (const SketchNode& node) {
        const glm::vec3& pos    = node.data ().position ();
        const float      radius = node.data ().radius ();

        this->nodeMesh.position (pos);
        this->nodeMesh.scaling  (glm::vec3 (radius));
        this->nodeMesh.color    (this->renderConfig.nodeColor);
        this->nodeMesh.render   (camera);

        if (node.parent ()) {
          const glm::vec3& parPos    = node.parent ()->data ().position ();
          const float      parRadius = node.parent ()->data ().radius ();
          const float      distance  = glm::distance (pos, parPos);
          const glm::vec3  direction = (parPos - pos) / distance;

          if (this->renderConfig.renderWireframe) {
            const glm::mat4x4 rotMatrix = glm::orientation ( direction
                                                           , glm::vec3 (0.0f, -1.0f, 0.0f) );

            this->wireframeMesh.color          (this->renderConfig.nodeColor);
            this->wireframeMesh.position       (parPos);
            this->wireframeMesh.scaling        (glm::vec3 (parRadius, distance, parRadius));
            this->wireframeMesh.rotationMatrix (rotMatrix);
            this->wireframeMesh.render         (camera);
          }
          else {
            this->nodeMesh.color (this->renderConfig.bubbleColor);

            for (float d = radius * 0.5f; d < distance; ) {
              const glm::vec3 bubblePos    = pos + (d * direction);
              const float     bubbleRadius = Util::lerp (d/distance, radius, parRadius);

              this->nodeMesh.position (bubblePos);
              this->nodeMesh.scaling  (glm::vec3 (bubbleRadius));
              this->nodeMesh.render   (camera);

              d += bubbleRadius * 0.5f;
            }
          }
        }
      });
    }
  }

  void renderWireframe (bool v) {
    this->renderConfig.renderWireframe = v;
  }

  void runFromConfig (const Config& config) {
    this->renderConfig.nodeColor   = config.get <Color> ("editor/sketch/node/color");
    this->renderConfig.bubbleColor = config.get <Color> ("editor/sketch/bubble/color");
  }
};

DELEGATE1_BIG3_SELF (SketchMesh, unsigned int);
DELEGATE1_CONST (bool              , SketchMesh, operator==, const SketchMesh&)
DELEGATE1_CONST (bool              , SketchMesh, operator!=, const SketchMesh&)
GETTER_CONST    (unsigned int      , SketchMesh, index)
DELEGATE_CONST  (bool              , SketchMesh, hasRoot)
DELEGATE        (SketchNode&       , SketchMesh, root)
DELEGATE_CONST  (const SketchNode& , SketchMesh, root)
DELEGATE1       (void              , SketchMesh, fromTree, const SketchNode&)
DELEGATE        (void              , SketchMesh, reset)
DELEGATE2       (bool              , SketchMesh, intersects, const PrimRay&, SketchNodeIntersection&)
DELEGATE1       (void              , SketchMesh, render, Camera&)
DELEGATE1       (void              , SketchMesh, renderWireframe, bool)
DELEGATE1       (void              , SketchMesh, runFromConfig, const Config&)
