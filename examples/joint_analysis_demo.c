/**
 * @file joint_analysis_demo.c
 * @brief 관절 분석 기능 데모 - JSON 데이터에서 중요 관절 자동 식별
 * @author Exercise Segment API Team
 * @version 2.1.0
 */

#include "../include/segment_api.h"
#include "../include/pose_analysis.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// 스쿼트 운동 시뮬레이션을 위한 포즈 생성
void create_squat_start_pose(PoseData *pose) {
  if (!pose) return;
  
  // 서있는 자세 (시작 포즈)
  *pose = (PoseData){0};
  pose->timestamp = 1000;
  
  // 기본 서있는 자세
  pose->landmarks[POSE_LANDMARK_NOSE] = (PoseLandmark){{400, 200, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_SHOULDER] = (PoseLandmark){{350, 300, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_SHOULDER] = (PoseLandmark){{450, 300, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_ELBOW] = (PoseLandmark){{320, 400, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_ELBOW] = (PoseLandmark){{480, 400, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_WRIST] = (PoseLandmark){{300, 500, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_WRIST] = (PoseLandmark){{500, 500, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_HIP] = (PoseLandmark){{380, 600, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_HIP] = (PoseLandmark){{420, 600, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_KNEE] = (PoseLandmark){{380, 800, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_KNEE] = (PoseLandmark){{420, 800, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_ANKLE] = (PoseLandmark){{380, 1000, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_ANKLE] = (PoseLandmark){{420, 1000, 0}, 0.9f};
}

void create_squat_end_pose(PoseData *pose) {
  if (!pose) return;
  
  // 앉은 자세 (종료 포즈) - 무릎이 많이 구부러짐
  *pose = (PoseData){0};
  pose->timestamp = 2000;
  
  // 앉은 자세
  pose->landmarks[POSE_LANDMARK_NOSE] = (PoseLandmark){{400, 200, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_SHOULDER] = (PoseLandmark){{350, 300, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_SHOULDER] = (PoseLandmark){{450, 300, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_ELBOW] = (PoseLandmark){{320, 400, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_ELBOW] = (PoseLandmark){{480, 400, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_WRIST] = (PoseLandmark){{300, 500, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_WRIST] = (PoseLandmark){{500, 500, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_HIP] = (PoseLandmark){{380, 600, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_HIP] = (PoseLandmark){{420, 600, 0}, 0.9f};
  // 무릎이 많이 구부러짐 (스쿼트의 핵심!)
  pose->landmarks[POSE_LANDMARK_LEFT_KNEE] = (PoseLandmark){{380, 950, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_KNEE] = (PoseLandmark){{420, 950, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_ANKLE] = (PoseLandmark){{380, 1000, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_ANKLE] = (PoseLandmark){{420, 1000, 0}, 0.9f};
}

void create_arm_exercise_start_pose(PoseData *pose) {
  if (!pose) return;
  
  // 팔굽혀펴기 시작 자세
  *pose = (PoseData){0};
  pose->timestamp = 1000;
  
  pose->landmarks[POSE_LANDMARK_NOSE] = (PoseLandmark){{400, 200, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_SHOULDER] = (PoseLandmark){{350, 300, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_SHOULDER] = (PoseLandmark){{450, 300, 0}, 0.9f};
  // 팔이 펴진 상태
  pose->landmarks[POSE_LANDMARK_LEFT_ELBOW] = (PoseLandmark){{350, 500, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_ELBOW] = (PoseLandmark){{450, 500, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_WRIST] = (PoseLandmark){{350, 700, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_WRIST] = (PoseLandmark){{450, 700, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_HIP] = (PoseLandmark){{380, 600, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_HIP] = (PoseLandmark){{420, 600, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_KNEE] = (PoseLandmark){{380, 800, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_KNEE] = (PoseLandmark){{420, 800, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_ANKLE] = (PoseLandmark){{380, 1000, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_ANKLE] = (PoseLandmark){{420, 1000, 0}, 0.9f};
}

void create_arm_exercise_end_pose(PoseData *pose) {
  if (!pose) return;
  
  // 팔굽혀펴기 종료 자세 - 팔꿈치가 많이 구부러짐
  *pose = (PoseData){0};
  pose->timestamp = 2000;
  
  pose->landmarks[POSE_LANDMARK_NOSE] = (PoseLandmark){{400, 200, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_SHOULDER] = (PoseLandmark){{350, 300, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_SHOULDER] = (PoseLandmark){{450, 300, 0}, 0.9f};
  // 팔꿈치가 많이 구부러짐 (팔굽혀펴기의 핵심!)
  pose->landmarks[POSE_LANDMARK_LEFT_ELBOW] = (PoseLandmark){{350, 400, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_ELBOW] = (PoseLandmark){{450, 400, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_WRIST] = (PoseLandmark){{350, 450, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_WRIST] = (PoseLandmark){{450, 450, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_HIP] = (PoseLandmark){{380, 600, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_HIP] = (PoseLandmark){{420, 600, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_KNEE] = (PoseLandmark){{380, 800, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_KNEE] = (PoseLandmark){{420, 800, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_ANKLE] = (PoseLandmark){{380, 1000, 0}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_ANKLE] = (PoseLandmark){{420, 1000, 0}, 0.9f};
}

int main() {
  printf("\n🔬 관절 분석 기능 데모\n");
  printf("========================================\n");
  printf("JSON 데이터에서 자동으로 중요 관절을 식별하는 기능을 보여줍니다.\n\n");

  // API 초기화
  printf("1. API 초기화 중...\n");
  int result = segment_api_init();
  if (result != SEGMENT_OK) {
    printf("❌ API 초기화 실패: %s\n", segment_get_error_message(result));
    return -1;
  }
  printf("✅ API 초기화 성공\n\n");

  // 사용자 캘리브레이션
  printf("2. 사용자 캘리브레이션 중...\n");
  PoseData base_pose;
  create_squat_start_pose(&base_pose);
  
  result = segment_calibrate_user(&base_pose);
  if (result != SEGMENT_OK) {
    printf("❌ 캘리브레이션 실패: %s\n", segment_get_error_message(result));
    return -1;
  }
  printf("✅ 캘리브레이션 성공\n\n");

  // 테스트 1: 스쿼트 운동 분석
  printf("🏋️‍♂️ 테스트 1: 스쿼트 운동 분석\n");
  printf("----------------------------------------\n");
  
  PoseData squat_start, squat_end;
  create_squat_start_pose(&squat_start);
  create_squat_end_pose(&squat_end);
  
  // 관절 분석 수행
  JointAnalysis joint_analysis[12];
  result = analyze_exercise_joints(&squat_start, &squat_end, joint_analysis);
  if (result == SEGMENT_OK) {
    print_important_joints(joint_analysis);
  }
  
  printf("\n");

  // 테스트 2: 팔굽혀펴기 운동 분석
  printf("💪 테스트 2: 팔굽혀펴기 운동 분석\n");
  printf("----------------------------------------\n");
  
  PoseData arm_start, arm_end;
  create_arm_exercise_start_pose(&arm_start);
  create_arm_exercise_end_pose(&arm_end);
  
  // 관절 분석 수행
  result = analyze_exercise_joints(&arm_start, &arm_end, joint_analysis);
  if (result == SEGMENT_OK) {
    print_important_joints(joint_analysis);
  }
  
  printf("\n");

  // 테스트 3: 실제 세그먼트 로드 및 분석
  printf("🎯 테스트 3: 실제 세그먼트 로드 및 분석\n");
  printf("----------------------------------------\n");
  
  // JSON 파일이 있다면 로드 시도
  printf("JSON 파일 로드 시도 중...\n");
  result = segment_load_all_segments("test_workout.json");
  if (result == SEGMENT_OK) {
    printf("✅ JSON 파일 로드 성공\n");
    
    // 세그먼트 설정 (자동으로 관절 분석 수행됨)
    result = segment_set_current_segment(0, 1);
    if (result == SEGMENT_OK) {
      printf("✅ 세그먼트 설정 완료 (관절 분석 자동 수행됨)\n");
    } else {
      printf("⚠️  세그먼트 설정 실패: %s\n", segment_get_error_message(result));
    }
  } else {
    printf("⚠️  JSON 파일 로드 실패: %s (테스트용 데이터로 계속 진행)\n", 
           segment_get_error_message(result));
  }

  printf("\n🎉 관절 분석 데모 완료!\n");
  printf("이제 시스템이 자동으로 어떤 관절이 중요한지 파악하고,\n");
  printf("그에 맞는 정확한 진행도 계산을 수행합니다.\n");

  // 정리
  segment_api_cleanup();
  return 0;
}
