/**
 * @file math_utils.c
 * @brief 수학 유틸리티 함수들 구현
 * @author Exercise Segment API Team
 * @version 1.0.0
 */

#include "math_utils.h"
#include <math.h>
#include <string.h>

float distance_2d(const Point2D* p1, const Point2D* p2) {
    if (!p1 || !p2) return 0.0f;
    
    float dx = p1->x - p2->x;
    float dy = p1->y - p2->y;
    
    return sqrtf(dx * dx + dy * dy);
}

float distance_3d(const Point3D* p1, const Point3D* p2) {
    if (!p1 || !p2) return 0.0f;
    
    float dx = p1->x - p2->x;
    float dy = p1->y - p2->y;
    float dz = p1->z - p2->z;
    
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

Point2D add_points(const Point2D* p1, const Point2D* p2) {
    Point2D result = {0.0f, 0.0f};
    
    if (p1 && p2) {
        result.x = p1->x + p2->x;
        result.y = p1->y + p2->y;
    }
    
    return result;
}

Point2D subtract_points(const Point2D* p1, const Point2D* p2) {
    Point2D result = {0.0f, 0.0f};
    
    if (p1 && p2) {
        result.x = p1->x - p2->x;
        result.y = p1->y - p2->y;
    }
    
    return result;
}

Point2D multiply_point(const Point2D* p, float scalar) {
    Point2D result = {0.0f, 0.0f};
    
    if (p) {
        result.x = p->x * scalar;
        result.y = p->y * scalar;
    }
    
    return result;
}

Point2D calculate_center_point(const PoseData* pose) {
    Point2D center = {0.0f, 0.0f};
    
    if (!pose) return center;
    
    int valid_joints = 0;
    for (int i = 0; i < JOINT_COUNT; i++) {
        if (pose->landmarks[i].inFrameLikelihood > 0.5f) {  // 신뢰도가 0.5 이상인 관절만 사용
            center.x += pose->landmarks[i].position.x;
            center.y += pose->landmarks[i].position.y;
            valid_joints++;
        }
    }
    
    if (valid_joints > 0) {
        center.x /= valid_joints;
        center.y /= valid_joints;
    }
    
    return center;
}

Point2D calculate_center_point_selected(const PoseData* pose, 
                                       const JointType* joints, 
                                       int joint_count) {
    Point2D center = {0.0f, 0.0f};
    
    if (!pose || !joints || joint_count <= 0) return center;
    
    int valid_joints = 0;
    for (int i = 0; i < joint_count; i++) {
        JointType joint = joints[i];
        if (joint >= 0 && joint < JOINT_COUNT && pose->landmarks[joint].inFrameLikelihood > 0.5f) {
            center.x += pose->landmarks[joint].position.x;
            center.y += pose->landmarks[joint].position.y;
            valid_joints++;
        }
    }
    
    if (valid_joints > 0) {
        center.x /= valid_joints;
        center.y /= valid_joints;
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
        result->landmarks[i].position.x = start->landmarks[i].position.x + t * (end->landmarks[i].position.x - start->landmarks[i].position.x);
        result->landmarks[i].position.y = start->landmarks[i].position.y + t * (end->landmarks[i].position.y - start->landmarks[i].position.y);
        
        // 신뢰도는 두 포즈의 평균으로 계산
        result->landmarks[i].inFrameLikelihood = (start->landmarks[i].inFrameLikelihood + end->landmarks[i].inFrameLikelihood) * 0.5f;
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
        result->landmarks[i].position.x = pose->landmarks[i].position.x * scale_factor;
        result->landmarks[i].position.y = pose->landmarks[i].position.y * scale_factor;
        result->landmarks[i].inFrameLikelihood = pose->landmarks[i].inFrameLikelihood;
    }
    result->timestamp = pose->timestamp;
}

void translate_pose(const PoseData* pose, const Point2D* offset, PoseData* result) {
    if (!pose || !offset || !result) return;
    
    for (int i = 0; i < JOINT_COUNT; i++) {
        result->landmarks[i].position.x = pose->landmarks[i].position.x + offset->x;
        result->landmarks[i].position.y = pose->landmarks[i].position.y + offset->y;
        result->landmarks[i].inFrameLikelihood = pose->landmarks[i].inFrameLikelihood;
    }
    result->timestamp = pose->timestamp;
}

void transform_pose(const PoseData* pose, 
                   float scale_factor, 
                   const Point2D* offset, 
                   PoseData* result) {
    if (!pose || !offset || !result) return;
    
    for (int i = 0; i < JOINT_COUNT; i++) {
        result->landmarks[i].position.x = pose->landmarks[i].position.x * scale_factor + offset->x;
        result->landmarks[i].position.y = pose->landmarks[i].position.y * scale_factor + offset->y;
        result->landmarks[i].inFrameLikelihood = pose->landmarks[i].inFrameLikelihood;
    }
    result->timestamp = pose->timestamp;
}
