#include <stdlib.h>
#include "mesh_inversion.h"
#include "maths.h"

bool flipped(unsigned int v1_id, unsigned int v2_id, Mesh* mesh, unsigned int* face, double* new_position) {
  double *vs[3], angle, *old_norm, *new_norm;
  unsigned int i, j, id[3];
  
  for (i = 0; i < 3; i++) {
    id[0] = face[i % 3];
    id[1] = face[(i + 1) % 3];
    id[2] = face[(i + 2) % 3];
    
    vs[0] = &(mesh->positions[id[0] * 3]);
    vs[1] = &(mesh->positions[id[1] * 3]);
    vs[2] = &(mesh->positions[id[2] * 3]);

    old_norm = normal(vs[0], vs[1], vs[2]);

    // replace old vertex by new one
    for (j = 0; j < 3; j++) {
      if (id[j] == v1_id || id[j] == v2_id) {
        vs[j] = new_position;
      }
    }

    new_norm = normal(vs[0], vs[1], vs[2]);

    // reset d
    old_norm[3] = 0;
    new_norm[3] = 0;

    angle = dot1d(old_norm, new_norm);
    free(old_norm);
    free(new_norm);

    if (angle < 0) {
      return true;
    }
  }

  return false;
  
}

bool has_mesh_inversion(unsigned int v1, unsigned int v2, Mesh* mesh, double* new_position, bool* deleted_faces) {
  unsigned int i, j;
  bool check_face, return_;

  return_ = false;

  #pragma omp parallel for shared(mesh, v1, v2, new_position, deleted_faces, return_) private(i, j, check_face)
  for (i = 0; i < mesh->n_face; i++) {
    if (deleted_faces[i] || return_) {
      continue;
    }

    check_face = false;

    for (j = 0; j < 3; j++) {
      if (mesh->face[i * 3 + j] == v1 || mesh->face[i * 3 + j] == v2) {
        check_face = true;
        break;
      }
    }

    if (check_face && flipped(v1, v2, mesh, &(mesh->face[i * 3]), new_position)) {
      return_ = true;
    }
  }

  return return_;
}