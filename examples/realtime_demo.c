/**
 * @file realtime_demo.c
 * @brief Exercise Segment API 실시간 데모 (ML Kit 33개 랜드마크 지원)
 * @author Exercise Segment API Team
 * @version 2.0.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "../include/segment_api.h"

// 샘플 포즈 데이터 생성 함수
void create_sample_pose(PoseData* pose, float offset_x, float offset_y, float offset_z) {
    if (!pose) return;
    
    // 기본 관절 위치 설정 (자연스러운 서있는 자세)
    // 주요 관절들만 설정 (ML Kit 33개 랜드마크 중 일부)
    pose->landmarks[POSE_LANDMARK_NOSE] = (PoseLandmark){{0.0f + offset_x, -10.0f + offset_y, 0.0f + offset_z}, 0.9f};
    pose->landmarks[POSE_LANDMARK_LEFT_SHOULDER] = (PoseLandmark){{-20.0f + offset_x, 0.0f + offset_y, 0.0f + offset_z}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_SHOULDER] = (PoseLandmark){{20.0f + offset_x, 0.0f + offset_y, 0.0f + offset_z}, 0.9f};
    pose->landmarks[POSE_LANDMARK_LEFT_ELBOW] = (PoseLandmark){{-30.0f + offset_x, 20.0f + offset_y, 0.0f + offset_z}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_ELBOW] = (PoseLandmark){{30.0f + offset_x, 20.0f + offset_y, 0.0f + offset_z}, 0.9f};
    pose->landmarks[POSE_LANDMARK_LEFT_WRIST] = (PoseLandmark){{-40.0f + offset_x, 40.0f + offset_y, 0.0f + offset_z}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_WRIST] = (PoseLandmark){{40.0f + offset_x, 40.0f + offset_y, 0.0f + offset_z}, 0.9f};
    pose->landmarks[POSE_LANDMARK_LEFT_HIP] = (PoseLandmark){{-10.0f + offset_x, 50.0f + offset_y, 0.0f + offset_z}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_HIP] = (PoseLandmark){{10.0f + offset_x, 50.0f + offset_y, 0.0f + offset_z}, 0.9f};
    pose->landmarks[POSE_LANDMARK_LEFT_KNEE] = (PoseLandmark){{-10.0f + offset_x, 80.0f + offset_y, 0.0f + offset_z}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_KNEE] = (PoseLandmark){{10.0f + offset_x, 80.0f + offset_y, 0.0f + offset_z}, 0.9f};
    pose->landmarks[POSE_LANDMARK_LEFT_ANKLE] = (PoseLandmark){{-10.0f + offset_x, 110.0f + offset_y, 0.0f + offset_z}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_ANKLE] = (PoseLandmark){{10.0f + offset_x, 110.0f + offset_y, 0.0f + offset_z}, 0.9f};
    
    // 나머지 랜드마크들은 기본값으로 설정
    for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
        if (pose->landmarks[i].inFrameLikelihood == 0.0f) {
            pose->landmarks[i] = (PoseLandmark){{0.0f, 0.0f, 0.0f}, 0.0f};
        }
    }
    
    pose->timestamp = (uint64_t)time(NULL) * 1000;  // 현재 시간 (밀리초)
}

// 스쿼트 시작 포즈 생성
void create_squat_start_pose(PoseData* pose) {
    create_sample_pose(pose, 0.0f, 0.0f, 0.0f);
}

// 스쿼트 끝 포즈 생성 (무릎을 구부린 상태)
void create_squat_end_pose(PoseData* pose) {
    create_sample_pose(pose, 0.0f, 0.0f, 0.0f);
    
    // 무릎과 골반을 아래로 이동
    pose->landmarks[POSE_LANDMARK_LEFT_KNEE].position.y += 30.0f;  // 무릎을 아래로
    pose->landmarks[POSE_LANDMARK_RIGHT_KNEE].position.y += 30.0f;
    pose->landmarks[POSE_LANDMARK_LEFT_HIP].position.y += 20.0f;   // 골반을 아래로
    pose->landmarks[POSE_LANDMARK_RIGHT_HIP].position.y += 20.0f;
}

// 두 포즈 간 보간
void interpolate_poses(const PoseData* start, const PoseData* end, float progress, PoseData* result) {
    if (!start || !end || !result) return;
    
    for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
        result->landmarks[i].position.x = start->landmarks[i].position.x + progress * (end->landmarks[i].position.x - start->landmarks[i].position.x);
        result->landmarks[i].position.y = start->landmarks[i].position.y + progress * (end->landmarks[i].position.y - start->landmarks[i].position.y);
        result->landmarks[i].position.z = start->landmarks[i].position.z + progress * (end->landmarks[i].position.z - start->landmarks[i].position.z);
        result->landmarks[i].inFrameLikelihood = start->landmarks[i].inFrameLikelihood;
    }
    result->timestamp = start->timestamp;
}

int main() {
    printf("=== Exercise Segment API 실시간 데모 (ML Kit 33개 랜드마크) ===\n\n");
    
    // 1. API 초기화
    printf("1. API 초기화 중...\n");
    int result = segment_api_init();
    if (result != SEGMENT_OK) {
        printf("❌ API 초기화 실패: %d\n", result);
        return 1;
    }
    printf("✅ API 초기화 성공\n\n");
    
    // 2. 캘리브레이션
    printf("2. 캘리브레이션 수행 중...\n");
    PoseData base_pose;
    create_sample_pose(&base_pose, 0.0f, 0.0f, 0.0f);
    
    result = segment_calibrate_recorder(&base_pose);
    if (result != SEGMENT_OK) {
        printf("❌ 캘리브레이션 실패: %d\n", result);
        segment_api_cleanup();
        return 1;
    }
    printf("✅ 캘리브레이션 성공\n\n");
    
    // 3. 실시간 스쿼트 운동 시뮬레이션
    printf("3. 실시간 스쿼트 운동 시뮬레이션...\n");
    printf("   (Ctrl+C로 종료)\n\n");
    
    PoseData start_pose, end_pose;
    create_squat_start_pose(&start_pose);
    create_squat_end_pose(&end_pose);
    
    int frame_count = 0;
    float direction = 1.0f;  // 1.0: 아래로, -1.0: 위로
    float current_progress = 0.0f;
    
    while (1) {
        // 운동 진행도 계산 (사인파 패턴)
        current_progress += direction * 0.05f;
        if (current_progress >= 1.0f) {
            current_progress = 1.0f;
            direction = -1.0f;  // 위로 올라가기
        } else if (current_progress <= 0.0f) {
            current_progress = 0.0f;
            direction = 1.0f;   // 아래로 내려가기
        }
        
        // 현재 포즈 생성
        PoseData current_pose;
        interpolate_poses(&start_pose, &end_pose, current_progress, &current_pose);
        
        // 분석 수행
        float out_progress, out_similarity;
        bool out_is_complete;
        Point3D out_corrections[POSE_LANDMARK_COUNT];
        
        result = segment_analyze_simple(&current_pose, &out_progress, &out_is_complete, &out_similarity, out_corrections);
        
        if (result == SEGMENT_OK) {
            // 진행률 바 생성
            int bar_length = 20;
            int filled_length = (int)(current_progress * bar_length);
            
            printf("\r프레임 %4d | 진행도: [", frame_count);
            for (int i = 0; i < bar_length; i++) {
                if (i < filled_length) {
                    printf("█");
                } else {
                    printf("░");
                }
            }
            printf("] %.1f%% | 완료: %s | 유사도: %.2f", 
                   current_progress * 100, out_is_complete ? "예" : "아니오", out_similarity);
            fflush(stdout);
        } else {
            printf("\r❌ 분석 실패: %d", result);
        }
        
        frame_count++;
        usleep(100000);  // 100ms 대기 (10 FPS)
    }
    
    printf("\n\n4. 정리 중...\n");
    segment_api_cleanup();
    printf("✅ 정리 완료\n");
    
    return 0;
}
