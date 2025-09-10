/**
 * @file basic_example.c
 * @brief Exercise Segment API 기본 사용 예제
 * @author Exercise Segment API Team
 * @version 1.0.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "segment_api.h"

// 샘플 포즈 데이터 생성 함수
void create_sample_pose(PoseData* pose, float offset_x, float offset_y, float offset_z) {
    if (!pose) return;
    
    // 기본 관절 위치 설정 (자연스러운 서있는 자세)
    pose->joints[JOINT_NOSE] = (Point3D){0.0f + offset_x, -10.0f + offset_y, 0.0f + offset_z};
    pose->joints[JOINT_LEFT_SHOULDER] = (Point3D){-20.0f + offset_x, 0.0f + offset_y, 0.0f + offset_z};
    pose->joints[JOINT_RIGHT_SHOULDER] = (Point3D){20.0f + offset_x, 0.0f + offset_y, 0.0f + offset_z};
    pose->joints[JOINT_LEFT_ELBOW] = (Point3D){-30.0f + offset_x, 20.0f + offset_y, 0.0f + offset_z};
    pose->joints[JOINT_RIGHT_ELBOW] = (Point3D){30.0f + offset_x, 20.0f + offset_y, 0.0f + offset_z};
    pose->joints[JOINT_LEFT_WRIST] = (Point3D){-40.0f + offset_x, 40.0f + offset_y, 0.0f + offset_z};
    pose->joints[JOINT_RIGHT_WRIST] = (Point3D){40.0f + offset_x, 40.0f + offset_y, 0.0f + offset_z};
    pose->joints[JOINT_LEFT_HIP] = (Point3D){-10.0f + offset_x, 50.0f + offset_y, 0.0f + offset_z};
    pose->joints[JOINT_RIGHT_HIP] = (Point3D){10.0f + offset_x, 50.0f + offset_y, 0.0f + offset_z};
    pose->joints[JOINT_LEFT_KNEE] = (Point3D){-10.0f + offset_x, 80.0f + offset_y, 0.0f + offset_z};
    pose->joints[JOINT_RIGHT_KNEE] = (Point3D){10.0f + offset_x, 80.0f + offset_y, 0.0f + offset_z};
    pose->joints[JOINT_LEFT_ANKLE] = (Point3D){-10.0f + offset_x, 110.0f + offset_y, 0.0f + offset_z};
    pose->joints[JOINT_RIGHT_ANKLE] = (Point3D){10.0f + offset_x, 110.0f + offset_y, 0.0f + offset_z};
    
    // 모든 관절의 신뢰도를 높게 설정
    for (int i = 0; i < JOINT_COUNT; i++) {
        pose->confidence[i] = 0.9f;
    }
    
    pose->timestamp = (uint64_t)time(NULL) * 1000;  // 현재 시간 (밀리초)
}

// 스쿼트 시작 포즈 생성
void create_squat_start_pose(PoseData* pose) {
    create_sample_pose(pose, 0.0f, 0.0f, 0.0f);
}

// 스쿼트 종료 포즈 생성 (무릎을 구부린 상태)
void create_squat_end_pose(PoseData* pose) {
    create_sample_pose(pose, 0.0f, 0.0f, 0.0f);
    
    // 무릎을 구부리고 골반을 낮춤
    pose->joints[JOINT_LEFT_KNEE].y += 30.0f;  // 무릎을 아래로
    pose->joints[JOINT_RIGHT_KNEE].y += 30.0f;
    pose->joints[JOINT_LEFT_HIP].y += 20.0f;   // 골반을 아래로
    pose->joints[JOINT_RIGHT_HIP].y += 20.0f;
}

// 중간 포즈 생성 (진행도에 따라)
void create_intermediate_pose(PoseData* pose, float progress) {
    PoseData start, end;
    create_squat_start_pose(&start);
    create_squat_end_pose(&end);
    
    // 선형 보간으로 중간 포즈 생성
    for (int i = 0; i < JOINT_COUNT; i++) {
        pose->joints[i].x = start.joints[i].x + progress * (end.joints[i].x - start.joints[i].x);
        pose->joints[i].y = start.joints[i].y + progress * (end.joints[i].y - start.joints[i].y);
        pose->joints[i].z = start.joints[i].z + progress * (end.joints[i].z - start.joints[i].z);
        pose->confidence[i] = 0.9f;
    }
    pose->timestamp = (uint64_t)time(NULL) * 1000;
}

int main() {
    printf("=== Exercise Segment API 기본 예제 ===\n\n");
    
    // 1. API 초기화
    printf("1. API 초기화 중...\n");
    int result = segment_api_init();
    if (result != SEGMENT_OK) {
        printf("초기화 실패: %s\n", segment_get_error_message(result));
        return -1;
    }
    printf("초기화 성공!\n\n");
    
    // 2. B 이용자 캘리브레이션
    printf("2. B 이용자 캘리브레이션 수행 중...\n");
    PoseData base_pose;
    create_sample_pose(&base_pose, 0.0f, 0.0f, 0.0f);
    
    result = segment_calibrate_user(&base_pose);
    if (result != SEGMENT_OK) {
        printf("캘리브레이션 실패: %s\n", segment_get_error_message(result));
        segment_api_cleanup();
        return -1;
    }
    printf("B 이용자 캘리브레이션 성공!\n\n");
    
    // 3. JSON 파일에서 세그먼트 로드 (더미 파일 사용)
    printf("3. 세그먼트 로드 중...\n");
    result = segment_load_segment("dummy_workout.json", 0, 1);
    if (result != SEGMENT_OK) {
        printf("세그먼트 로드 실패: %s\n", segment_get_error_message(result));
        segment_api_cleanup();
        return -1;
    }
    printf("세그먼트 로드 성공!\n\n");
    
    // 4. 실시간 분석 시뮬레이션
    printf("4. 실시간 분석 시뮬레이션...\n");
    printf("진행도 | 완료 | 유사도 | 교정 벡터 (무릎, 골반)\n");
    printf("-------|------|--------|----------------------\n");
    
    // 다양한 진행도로 테스트
    float test_progresses[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
    int num_tests = sizeof(test_progresses) / sizeof(test_progresses[0]);
    
    for (int i = 0; i < num_tests; i++) {
        float progress = test_progresses[i];
        
        // 중간 포즈 생성
        PoseData current_pose;
        create_intermediate_pose(&current_pose, progress);
        
        // 분석 수행
        SegmentOutput output = segment_analyze(&current_pose);
        
        // 결과 출력
        printf("%.2f   | %s   | %.2f   | ", 
               output.progress, 
               output.completed ? "예" : "아니오",
               output.similarity);
        
        // 관절별 교정 벡터 출력 (무릎과 골반)
        JointType care_joints[] = {JOINT_LEFT_KNEE, JOINT_RIGHT_KNEE, JOINT_LEFT_HIP, JOINT_RIGHT_HIP};
        for (int j = 0; j < 4; j++) {
            JointType joint = care_joints[j];
            Point3D correction = output.corrections[joint];
            printf("(%.1f,%.1f,%.1f) ", correction.x, correction.y, correction.z);
        }
        printf("\n");
    }
    
    printf("\n");
    
    // 5. 포즈 유효성 검사 테스트
    printf("5. 포즈 유효성 검사 테스트...\n");
    PoseData valid_pose, invalid_pose;
    create_sample_pose(&valid_pose, 0.0f, 0.0f, 0.0f);
    
    // 무효한 포즈 생성 (신뢰도가 낮은 포즈)
    create_sample_pose(&invalid_pose, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < JOINT_COUNT; i++) {
        invalid_pose.confidence[i] = 0.3f;  // 낮은 신뢰도
    }
    
    bool valid_result = segment_validate_pose(&valid_pose);
    bool invalid_result = segment_validate_pose(&invalid_pose);
    
    printf("유효한 포즈 검사: %s\n", valid_result ? "통과" : "실패");
    printf("무효한 포즈 검사: %s\n", !invalid_result ? "통과" : "실패");
    
    // 6. 정리
    printf("\n6. 정리 중...\n");
    segment_destroy();
    segment_api_cleanup();
    printf("정리 완료!\n");
    
    printf("\n=== 예제 완료 ===\n");
    return 0;
}
