/**
 * @file test_math_utils.c
 * @brief 수학 유틸리티 함수들 테스트
 * @author Exercise Segment API Team
 * @version 1.0.0
 */

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "math_utils.h"

#define EPSILON 0.001f

void test_distance_3d() {
    printf("테스트: distance_3d\n");
    
    Point3D p1 = {0.0f, 0.0f, 0.0f};
    Point3D p2 = {3.0f, 4.0f, 0.0f};
    
    float distance = distance_3d(&p1, &p2);
    assert(fabsf(distance - 5.0f) < EPSILON);
    
    printf("  ✓ 거리 계산 테스트 통과\n");
}

void test_add_points() {
    printf("테스트: add_points\n");
    
    Point3D p1 = {1.0f, 2.0f, 3.0f};
    Point3D p2 = {4.0f, 5.0f, 6.0f};
    
    Point3D result = add_points(&p1, &p2);
    assert(fabsf(result.x - 5.0f) < EPSILON);
    assert(fabsf(result.y - 7.0f) < EPSILON);
    assert(fabsf(result.z - 9.0f) < EPSILON);
    
    printf("  ✓ 점 덧셈 테스트 통과\n");
}

void test_subtract_points() {
    printf("테스트: subtract_points\n");
    
    Point3D p1 = {5.0f, 7.0f, 9.0f};
    Point3D p2 = {1.0f, 2.0f, 3.0f};
    
    Point3D result = subtract_points(&p1, &p2);
    assert(fabsf(result.x - 4.0f) < EPSILON);
    assert(fabsf(result.y - 5.0f) < EPSILON);
    assert(fabsf(result.z - 6.0f) < EPSILON);
    
    printf("  ✓ 점 뺄셈 테스트 통과\n");
}

void test_multiply_point() {
    printf("테스트: multiply_point\n");
    
    Point3D p = {2.0f, 3.0f, 4.0f};
    float scalar = 2.5f;
    
    Point3D result = multiply_point(&p, scalar);
    assert(fabsf(result.x - 5.0f) < EPSILON);
    assert(fabsf(result.y - 7.5f) < EPSILON);
    assert(fabsf(result.z - 10.0f) < EPSILON);
    
    printf("  ✓ 스칼라 곱셈 테스트 통과\n");
}

void test_calculate_center_point() {
    printf("테스트: calculate_center_point\n");
    
    PoseData pose;
    pose.joints[0] = (Point3D){0.0f, 0.0f, 0.0f};
    pose.joints[1] = (Point3D){10.0f, 0.0f, 0.0f};
    pose.joints[2] = (Point3D){0.0f, 10.0f, 0.0f};
    pose.joints[3] = (Point3D){10.0f, 10.0f, 0.0f};
    
    // 처음 4개 관절만 유효하게 설정
    for (int i = 0; i < 4; i++) {
        pose.confidence[i] = 0.9f;
    }
    for (int i = 4; i < JOINT_COUNT; i++) {
        pose.confidence[i] = 0.0f;
    }
    
    Point3D center = calculate_center_point(&pose);
    assert(fabsf(center.x - 5.0f) < EPSILON);
    assert(fabsf(center.y - 5.0f) < EPSILON);
    assert(fabsf(center.z - 0.0f) < EPSILON);
    
    printf("  ✓ 중심점 계산 테스트 통과\n");
}

void test_interpolate_pose() {
    printf("테스트: interpolate_pose\n");
    
    PoseData start, end, result;
    
    // 시작 포즈
    start.joints[0] = (Point3D){0.0f, 0.0f, 0.0f};
    start.confidence[0] = 0.8f;
    start.timestamp = 1000;
    
    // 종료 포즈
    end.joints[0] = (Point3D){10.0f, 10.0f, 10.0f};
    end.confidence[0] = 0.9f;
    end.timestamp = 2000;
    
    // 중간 지점 보간 (t=0.5)
    interpolate_pose(&start, &end, 0.5f, &result);
    
    assert(fabsf(result.joints[0].x - 5.0f) < EPSILON);
    assert(fabsf(result.joints[0].y - 5.0f) < EPSILON);
    assert(fabsf(result.joints[0].z - 5.0f) < EPSILON);
    assert(fabsf(result.confidence[0] - 0.85f) < EPSILON);
    assert(result.timestamp == 1500);
    
    printf("  ✓ 포즈 보간 테스트 통과\n");
}

void test_clamp() {
    printf("테스트: clamp\n");
    
    assert(fabsf(clamp(5.0f, 0.0f, 10.0f) - 5.0f) < EPSILON);
    assert(fabsf(clamp(-5.0f, 0.0f, 10.0f) - 0.0f) < EPSILON);
    assert(fabsf(clamp(15.0f, 0.0f, 10.0f) - 10.0f) < EPSILON);
    
    printf("  ✓ 값 제한 테스트 통과\n");
}

void test_fast_sqrt() {
    printf("테스트: fast_sqrt\n");
    
    float test_values[] = {0.0f, 1.0f, 4.0f, 9.0f, 16.0f, 25.0f};
    float expected_values[] = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    
    for (int i = 0; i < 6; i++) {
        float result = fast_sqrt(test_values[i]);
        assert(fabsf(result - expected_values[i]) < 0.1f);  // 빠른 근사이므로 오차 허용
    }
    
    printf("  ✓ 빠른 제곱근 테스트 통과\n");
}

void test_scale_pose() {
    printf("테스트: scale_pose\n");
    
    PoseData original, result;
    original.joints[0] = (Point3D){2.0f, 3.0f, 4.0f};
    original.confidence[0] = 0.8f;
    original.timestamp = 1000;
    
    scale_pose(&original, 2.0f, &result);
    
    assert(fabsf(result.joints[0].x - 4.0f) < EPSILON);
    assert(fabsf(result.joints[0].y - 6.0f) < EPSILON);
    assert(fabsf(result.joints[0].z - 8.0f) < EPSILON);
    assert(fabsf(result.confidence[0] - 0.8f) < EPSILON);
    assert(result.timestamp == 1000);
    
    printf("  ✓ 포즈 스케일링 테스트 통과\n");
}

void test_translate_pose() {
    printf("테스트: translate_pose\n");
    
    PoseData original, result;
    Point3D offset = {1.0f, 2.0f, 3.0f};
    
    original.joints[0] = (Point3D){5.0f, 6.0f, 7.0f};
    original.confidence[0] = 0.8f;
    original.timestamp = 1000;
    
    translate_pose(&original, &offset, &result);
    
    assert(fabsf(result.joints[0].x - 6.0f) < EPSILON);
    assert(fabsf(result.joints[0].y - 8.0f) < EPSILON);
    assert(fabsf(result.joints[0].z - 10.0f) < EPSILON);
    assert(fabsf(result.confidence[0] - 0.8f) < EPSILON);
    assert(result.timestamp == 1000);
    
    printf("  ✓ 포즈 이동 테스트 통과\n");
}

int main() {
    printf("=== 수학 유틸리티 함수 테스트 ===\n\n");
    
    test_distance_3d();
    test_add_points();
    test_subtract_points();
    test_multiply_point();
    test_calculate_center_point();
    test_interpolate_pose();
    test_clamp();
    test_fast_sqrt();
    test_scale_pose();
    test_translate_pose();
    
    printf("\n=== 모든 테스트 통과! ===\n");
    return 0;
}
