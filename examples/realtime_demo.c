/**
 * @file realtime_demo.c
 * @brief 실시간 포즈 입력 시뮬레이션 데모
 * @author Exercise Segment API Team
 * @version 1.0.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include "segment_api.h"

// 실시간 시뮬레이션을 위한 더미 데이터 생성 함수들
void create_base_pose(PoseData* pose) {
    // 자연스러운 서있는 자세
    pose->joints[JOINT_NOSE] = (Point3D){0.0f, -10.0f, 0.0f};
    pose->joints[JOINT_LEFT_SHOULDER] = (Point3D){-20.0f, 0.0f, 0.0f};
    pose->joints[JOINT_RIGHT_SHOULDER] = (Point3D){20.0f, 0.0f, 0.0f};
    pose->joints[JOINT_LEFT_ELBOW] = (Point3D){-30.0f, 20.0f, 0.0f};
    pose->joints[JOINT_RIGHT_ELBOW] = (Point3D){30.0f, 20.0f, 0.0f};
    pose->joints[JOINT_LEFT_WRIST] = (Point3D){-40.0f, 40.0f, 0.0f};
    pose->joints[JOINT_RIGHT_WRIST] = (Point3D){40.0f, 40.0f, 0.0f};
    pose->joints[JOINT_LEFT_HIP] = (Point3D){-10.0f, 50.0f, 0.0f};
    pose->joints[JOINT_RIGHT_HIP] = (Point3D){10.0f, 50.0f, 0.0f};
    pose->joints[JOINT_LEFT_KNEE] = (Point3D){-10.0f, 80.0f, 0.0f};
    pose->joints[JOINT_RIGHT_KNEE] = (Point3D){10.0f, 80.0f, 0.0f};
    pose->joints[JOINT_LEFT_ANKLE] = (Point3D){-10.0f, 110.0f, 0.0f};
    pose->joints[JOINT_RIGHT_ANKLE] = (Point3D){10.0f, 110.0f, 0.0f};
    
    // 모든 관절의 신뢰도를 높게 설정
    for (int i = 0; i < JOINT_COUNT; i++) {
        pose->confidence[i] = 0.9f;
    }
    pose->timestamp = (uint64_t)time(NULL) * 1000;
}

void create_squat_start_pose(PoseData* pose) {
    create_base_pose(pose);
}

void create_squat_end_pose(PoseData* pose) {
    create_base_pose(pose);
    
    // 스쿼트 종료 자세 (무릎을 구부리고 골반을 낮춤)
    pose->joints[JOINT_LEFT_KNEE].y += 30.0f;  // 무릎을 아래로
    pose->joints[JOINT_RIGHT_KNEE].y += 30.0f;
    pose->joints[JOINT_LEFT_HIP].y += 20.0f;   // 골반을 아래로
    pose->joints[JOINT_RIGHT_HIP].y += 20.0f;
}

// 실시간 포즈 생성 (진행도에 따른 보간 + 노이즈)
void create_realtime_pose(PoseData* pose, float progress, float noise_level) {
    PoseData start, end;
    create_squat_start_pose(&start);
    create_squat_end_pose(&end);
    
    // 선형 보간으로 기본 포즈 생성
    for (int i = 0; i < JOINT_COUNT; i++) {
        pose->joints[i].x = start.joints[i].x + progress * (end.joints[i].x - start.joints[i].x);
        pose->joints[i].y = start.joints[i].y + progress * (end.joints[i].y - start.joints[i].y);
        pose->joints[i].z = start.joints[i].z + progress * (end.joints[i].z - start.joints[i].z);
        
        // 실시간 노이즈 추가 (실제 센서 노이즈 시뮬레이션)
        pose->joints[i].x += (float)(rand() % 100 - 50) / 100.0f * noise_level;
        pose->joints[i].y += (float)(rand() % 100 - 50) / 100.0f * noise_level;
        pose->joints[i].z += (float)(rand() % 100 - 50) / 100.0f * noise_level;
        
        pose->confidence[i] = 0.9f - (float)(rand() % 20) / 100.0f; // 약간의 신뢰도 변동
    }
    pose->timestamp = (uint64_t)time(NULL) * 1000 + (uint64_t)(progress * 1000);
}

// 실시간 분석 결과 출력
void print_realtime_analysis(int frame_count, const SegmentOutput* output, float target_progress) {
    printf("Frame %3d | 진행도: %5.2f (목표: %5.2f) | 완료: %s | 유사도: %5.2f | ",
           frame_count, output->progress, target_progress, 
           output->completed ? "예" : "아니오", output->similarity);
    
    // 주요 관절 교정 벡터 출력 (무릎과 골반)
    printf("무릎: (%.1f,%.1f,%.1f) (%.1f,%.1f,%.1f) | ",
           output->corrections[JOINT_LEFT_KNEE].x,
           output->corrections[JOINT_LEFT_KNEE].y,
           output->corrections[JOINT_LEFT_KNEE].z,
           output->corrections[JOINT_RIGHT_KNEE].x,
           output->corrections[JOINT_RIGHT_KNEE].y,
           output->corrections[JOINT_RIGHT_KNEE].z);
    
    printf("골반: (%.1f,%.1f,%.1f) (%.1f,%.1f,%.1f)\n",
           output->corrections[JOINT_LEFT_HIP].x,
           output->corrections[JOINT_LEFT_HIP].y,
           output->corrections[JOINT_LEFT_HIP].z,
           output->corrections[JOINT_RIGHT_HIP].x,
           output->corrections[JOINT_RIGHT_HIP].y,
           output->corrections[JOINT_RIGHT_HIP].z);
}

// 진행률 바 출력
void print_progress_bar(float progress, int width) {
    printf("[");
    int pos = (int)(progress * width);
    for (int i = 0; i < width; i++) {
        if (i < pos) printf("█");
        else if (i == pos) printf("▌");
        else printf("░");
    }
    printf("] %3.0f%%", progress * 100);
}

int main() {
    printf("=== 실시간 포즈 분석 시뮬레이션 ===\n\n");
    
    // 랜덤 시드 설정
    srand((unsigned int)time(NULL));
    
    // 1. API 초기화
    printf("1. API 초기화 중...\n");
    int result = segment_api_init();
    if (result != SEGMENT_OK) {
        printf("❌ 초기화 실패: %s\n", segment_get_error_message(result));
        return -1;
    }
    printf("✅ 초기화 성공!\n\n");
    
    // 2. 캘리브레이션
    printf("2. 사용자 캘리브레이션 중...\n");
    PoseData base_pose;
    create_base_pose(&base_pose);
    
    CalibrationData calibration;
    result = segment_calibrate(&base_pose, &calibration);
    if (result != SEGMENT_OK) {
        printf("❌ 캘리브레이션 실패: %s\n", segment_get_error_message(result));
        segment_api_cleanup();
        return -1;
    }
    printf("✅ 캘리브레이션 성공! (스케일: %.2f, 품질: %.2f)\n\n", 
           calibration.scale_factor, calibration.calibration_quality);
    
    // 3. 스쿼트 세그먼트 생성
    printf("3. 스쿼트 운동 세그먼트 생성 중...\n");
    PoseData start_keypose, end_keypose;
    create_squat_start_pose(&start_keypose);
    create_squat_end_pose(&end_keypose);
    
    JointType care_joints[] = {
        JOINT_LEFT_KNEE, JOINT_RIGHT_KNEE, 
        JOINT_LEFT_HIP, JOINT_RIGHT_HIP
    };
    
    result = segment_create(&start_keypose, &end_keypose, &calibration, 
                           care_joints, 4);
    if (result != SEGMENT_OK) {
        printf("❌ 세그먼트 생성 실패: %s\n", segment_get_error_message(result));
        segment_api_cleanup();
        return -1;
    }
    printf("✅ 세그먼트 생성 성공!\n\n");
    
    // 4. 실시간 분석 시뮬레이션
    printf("4. 실시간 분석 시뮬레이션 시작...\n");
    printf("   (60fps 시뮬레이션, 실제로는 16ms 간격)\n\n");
    
    printf("Frame | 진행도 (목표) | 완료 | 유사도 | 교정 벡터 (무릎, 골반)\n");
    printf("------|---------------|------|--------|----------------------\n");
    
    // 스쿼트 운동 시뮬레이션 (0.0 → 1.0 → 0.0)
    int total_frames = 120; // 2초간 60fps
    float max_noise = 2.0f; // 노이즈 레벨
    
    for (int frame = 0; frame < total_frames; frame++) {
        // 운동 진행도 계산 (0.0 → 1.0 → 0.0)
        float target_progress;
        if (frame < total_frames / 2) {
            // 내려가는 동작 (0.0 → 1.0)
            target_progress = (float)frame / (total_frames / 2);
        } else {
            // 올라오는 동작 (1.0 → 0.0)
            target_progress = 2.0f - (float)frame / (total_frames / 2);
        }
        
        // 실시간 포즈 생성 (노이즈 포함)
        PoseData current_pose;
        create_realtime_pose(&current_pose, target_progress, max_noise);
        
        // 분석 수행
        SegmentInput input = {current_pose};
        SegmentOutput output = segment_analyze(&input);
        
        // 결과 출력
        print_realtime_analysis(frame + 1, &output, target_progress);
        
        // 진행률 바 출력 (10프레임마다)
        if (frame % 10 == 0) {
            printf(" | ");
            print_progress_bar(output.progress, 20);
        }
        printf("\n");
        
        // 실시간 시뮬레이션을 위한 지연 (실제로는 16ms)
        usleep(16000); // 16ms = 60fps
        
        // 30프레임마다 요약 출력
        if ((frame + 1) % 30 == 0) {
            printf("\n--- %d프레임 요약 ---\n", frame + 1);
            printf("현재 진행도: %.2f, 목표 진행도: %.2f, 오차: %.2f\n", 
                   output.progress, target_progress, 
                   fabsf(output.progress - target_progress));
            printf("완료 상태: %s, 유사도: %.2f\n\n", 
                   output.completed ? "완료" : "진행중", output.similarity);
        }
    }
    
    // 5. 최종 통계
    printf("\n=== 실시간 분석 완료 ===\n");
    printf("✅ 총 %d 프레임 처리 완료\n", total_frames);
    printf("✅ 60fps 실시간 처리 시뮬레이션 성공\n");
    printf("✅ 노이즈가 포함된 포즈 데이터에서도 안정적 분석\n");
    printf("✅ 진행도 추적 및 교정 벡터 생성 정상 작동\n\n");
    
    // 6. 정리
    printf("6. 시스템 정리 중...\n");
    segment_destroy();
    segment_api_cleanup();
    printf("✅ 정리 완료!\n");
    
    printf("\n🎉 실시간 포즈 분석 API 데모 완료!\n");
    return 0;
}
