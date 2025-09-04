/**
 * @file math_utils.c
 * @brief 수학 유틸리티 함수들 구현
 * @author Exercise Segment API Team
 * @version 1.0.0
 */

#include "math_utils.h"
#include <math.h>
#include <string.h>

float distance_3d(const Point3D* p1, const Point3D* p2) {
    if (!p1 || !p2) return 0.0f;
    
    float dx = p1->x - p2->x;
    float dy = p1->y - p2->y;
    float dz = p1->z - p2->z;
    
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

Point3D add_points(const Point3D* p1, const Point3D* p2) {
    Point3D result = {0.0f, 0.0f, 0.0f};
    
    if (p1 && p2) {
        result.x = p1->x + p2->x;
        result.y = p1->y + p2->y;
        result.z = p1->z + p2->z;
    }
    
    return result;
}

Point3D subtract_points(const Point3D* p1, const Point3D* p2) {
    Point3D result = {0.0f, 0.0f, 0.0f};
    
    if (p1 && p2) {
        result.x = p1->x - p2->x;
        result.y = p1->y - p2->y;
        result.z = p1->z - p2->z;
    }
    
    return result;
}

Point3D multiply_point(const Point3D* p, float scalar) {
    Point3D result = {0.0f, 0.0f, 0.0f};
    
    if (p) {
        result.x = p->x * scalar;
        result.y = p->y * scalar;
        result.z = p->z * scalar;
    }
    
    return result;
}

Point3D calculate_center_point(const PoseData* pose) {
    Point3D center = {0.0f, 0.0f, 0.0f};
    
    if (!pose) return center;
    
    int valid_joints = 0;
    for (int i = 0; i < JOINT_COUNT; i++) {
        if (pose->confidence[i] > 0.5f) {  // 신뢰도가 0.5 이상인 관절만 사용
            center.x += pose->joints[i].x;
            center.y += pose->joints[i].y;
            center.z += pose->joints[i].z;
            valid_joints++;
        }
    }
    
    if (valid_joints > 0) {
        center.x /= valid_joints;
        center.y /= valid_joints;
        center.z /= valid_joints;
    }
    
    return center;
}

Point3D calculate_center_point_selected(const PoseData* pose, 
                                       const JointType* joints, 
                                       int joint_count) {
    Point3D center = {0.0f, 0.0f, 0.0f};
    
    if (!pose || !joints || joint_count <= 0) return center;
    
    int valid_joints = 0;
    for (int i = 0; i < joint_count; i++) {
        JointType joint = joints[i];
        if (joint >= 0 && joint < JOINT_COUNT && pose->confidence[joint] > 0.5f) {
            center.x += pose->joints[joint].x;
            center.y += pose->joints[joint].y;
            center.z += pose->joints[joint].z;
            valid_joints++;
        }
    }
    
    if (valid_joints > 0) {
        center.x /= valid_joints;
        center.y /= valid_joints;
        center.z /= valid_joints;
    }
    
    return center;
}

void interpolate_pose(const PoseData* start, 
                     const PoseData* end, 
                     float t, 
                     PoseData* result) {
    if (!start || !end || !result) return;
    
    t = clamp(t, 0.0f, 1.0f);
    
    for (int i = 0; i < JOINT_COUNT; i++) {
        result->joints[i].x = start->joints[i].x + t * (end->joints[i].x - start->joints[i].x);
        result->joints[i].y = start->joints[i].y + t * (end->joints[i].y - start->joints[i].y);
        result->joints[i].z = start->joints[i].z + t * (end->joints[i].z - start->joints[i].z);
        
        // 신뢰도는 두 포즈의 평균으로 계산
        result->confidence[i] = (start->confidence[i] + end->confidence[i]) * 0.5f;
    }
    
    result->timestamp = start->timestamp + (uint64_t)(t * (end->timestamp - start->timestamp));
}

float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

float fast_sqrt(float x) {
    if (x < 0.0f) return 0.0f;
    if (x == 0.0f) return 0.0f;
    if (x == 1.0f) return 1.0f;
    
    // 표준 라이브러리 sqrtf 사용 (실제로는 충분히 빠름)
    return sqrtf(x);
}

void scale_pose(const PoseData* pose, float scale_factor, PoseData* result) {
    if (!pose || !result) return;
    
    for (int i = 0; i < JOINT_COUNT; i++) {
        result->joints[i].x = pose->joints[i].x * scale_factor;
        result->joints[i].y = pose->joints[i].y * scale_factor;
        result->joints[i].z = pose->joints[i].z * scale_factor;
        result->confidence[i] = pose->confidence[i];
    }
    result->timestamp = pose->timestamp;
}

void translate_pose(const PoseData* pose, const Point3D* offset, PoseData* result) {
    if (!pose || !offset || !result) return;
    
    for (int i = 0; i < JOINT_COUNT; i++) {
        result->joints[i].x = pose->joints[i].x + offset->x;
        result->joints[i].y = pose->joints[i].y + offset->y;
        result->joints[i].z = pose->joints[i].z + offset->z;
        result->confidence[i] = pose->confidence[i];
    }
    result->timestamp = pose->timestamp;
}

void transform_pose(const PoseData* pose, 
                   float scale_factor, 
                   const Point3D* offset, 
                   PoseData* result) {
    if (!pose || !offset || !result) return;
    
    for (int i = 0; i < JOINT_COUNT; i++) {
        result->joints[i].x = pose->joints[i].x * scale_factor + offset->x;
        result->joints[i].y = pose->joints[i].y * scale_factor + offset->y;
        result->joints[i].z = pose->joints[i].z * scale_factor + offset->z;
        result->confidence[i] = pose->confidence[i];
    }
    result->timestamp = pose->timestamp;
}
