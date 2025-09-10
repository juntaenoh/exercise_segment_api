/**
 * @file basic_example.c
 * @brief Exercise Segment API 기본 사용 예제 (ML Kit 33개 랜드마크 지원)
 * @author Exercise Segment API Team
 * @version 2.0.0
 */

#include "../include/segment_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// 샘플 포즈 데이터 생성 함수
void create_sample_pose(PoseData *pose, float offset_x, float offset_y,
                        float offset_z) {
  if (!pose)
    return;

  // 모든 33개 랜드마크를 기본값으로 초기화
  for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
    pose->landmarks[i] = (PoseLandmark){{0.0f, 0.0f, 0.0f}, 0.0f};
  }

  // 주요 관절 위치 설정 (자연스러운 서있는 자세)
  // 얼굴 (11개)
  pose->landmarks[POSE_LANDMARK_NOSE] = (PoseLandmark){
      {0.0f + offset_x, -10.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_EYE_INNER] = (PoseLandmark){
      {-2.0f + offset_x, -8.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_EYE] = (PoseLandmark){
      {-4.0f + offset_x, -8.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_EYE_OUTER] = (PoseLandmark){
      {-6.0f + offset_x, -8.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_EYE_INNER] = (PoseLandmark){
      {2.0f + offset_x, -8.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_EYE] = (PoseLandmark){
      {4.0f + offset_x, -8.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_EYE_OUTER] = (PoseLandmark){
      {6.0f + offset_x, -8.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_EAR] = (PoseLandmark){
      {-8.0f + offset_x, -6.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_EAR] = (PoseLandmark){
      {8.0f + offset_x, -6.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_MOUTH_LEFT] = (PoseLandmark){
      {-3.0f + offset_x, -5.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_MOUTH_RIGHT] = (PoseLandmark){
      {3.0f + offset_x, -5.0f + offset_y, 0.0f + offset_z}, 0.9f};

  // 상체 (12개)
  pose->landmarks[POSE_LANDMARK_LEFT_SHOULDER] = (PoseLandmark){
      {-20.0f + offset_x, 0.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_SHOULDER] = (PoseLandmark){
      {20.0f + offset_x, 0.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_ELBOW] = (PoseLandmark){
      {-30.0f + offset_x, 20.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_ELBOW] = (PoseLandmark){
      {30.0f + offset_x, 20.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_WRIST] = (PoseLandmark){
      {-40.0f + offset_x, 40.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_WRIST] = (PoseLandmark){
      {40.0f + offset_x, 40.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_PINKY] = (PoseLandmark){
      {-42.0f + offset_x, 38.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_PINKY] = (PoseLandmark){
      {42.0f + offset_x, 38.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_INDEX] = (PoseLandmark){
      {-38.0f + offset_x, 38.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_INDEX] = (PoseLandmark){
      {38.0f + offset_x, 38.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_THUMB] = (PoseLandmark){
      {-36.0f + offset_x, 36.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_THUMB] = (PoseLandmark){
      {36.0f + offset_x, 36.0f + offset_y, 0.0f + offset_z}, 0.9f};

  // 하체 (10개)
  pose->landmarks[POSE_LANDMARK_LEFT_HIP] = (PoseLandmark){
      {-10.0f + offset_x, 50.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_HIP] = (PoseLandmark){
      {10.0f + offset_x, 50.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_KNEE] = (PoseLandmark){
      {-10.0f + offset_x, 80.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_KNEE] = (PoseLandmark){
      {10.0f + offset_x, 80.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_ANKLE] = (PoseLandmark){
      {-10.0f + offset_x, 110.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_ANKLE] = (PoseLandmark){
      {10.0f + offset_x, 110.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_HEEL] = (PoseLandmark){
      {-12.0f + offset_x, 112.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_HEEL] = (PoseLandmark){
      {12.0f + offset_x, 112.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_FOOT_INDEX] = (PoseLandmark){
      {-8.0f + offset_x, 112.0f + offset_y, 0.0f + offset_z}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_FOOT_INDEX] = (PoseLandmark){
      {8.0f + offset_x, 112.0f + offset_y, 0.0f + offset_z}, 0.9f};

  pose->timestamp = (uint64_t)time(NULL) * 1000; // 현재 시간 (밀리초)
}

// 스쿼트 시작 포즈 생성
void create_squat_start_pose(PoseData *pose) {
  create_sample_pose(pose, 0.0f, 0.0f, 0.0f);
}

// 스쿼트 끝 포즈 생성 (무릎을 구부린 상태)
void create_squat_end_pose(PoseData *pose) {
  create_sample_pose(pose, 0.0f, 0.0f, 0.0f);

  // 무릎과 골반을 아래로 이동
  pose->landmarks[POSE_LANDMARK_LEFT_KNEE].position.y += 30.0f; // 무릎을 아래로
  pose->landmarks[POSE_LANDMARK_RIGHT_KNEE].position.y += 30.0f;
  pose->landmarks[POSE_LANDMARK_LEFT_HIP].position.y += 20.0f; // 골반을 아래로
  pose->landmarks[POSE_LANDMARK_RIGHT_HIP].position.y += 20.0f;
}

// 두 포즈 간 보간
void interpolate_poses(const PoseData *start, const PoseData *end,
                       float progress, PoseData *result) {
  if (!start || !end || !result)
    return;

  for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
    result->landmarks[i].position.x =
        start->landmarks[i].position.x +
        progress *
            (end->landmarks[i].position.x - start->landmarks[i].position.x);
    result->landmarks[i].position.y =
        start->landmarks[i].position.y +
        progress *
            (end->landmarks[i].position.y - start->landmarks[i].position.y);
    result->landmarks[i].position.z =
        start->landmarks[i].position.z +
        progress *
            (end->landmarks[i].position.z - start->landmarks[i].position.z);
    result->landmarks[i].inFrameLikelihood =
        start->landmarks[i].inFrameLikelihood;
  }
  result->timestamp = start->timestamp;
}

int main() {
  printf("=== Exercise Segment API 기본 예제 (ML Kit 33개 랜드마크) ===\n\n");

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

  // 3. 포즈 기록 테스트 (A 이용자)
  printf("3. 포즈 기록 테스트...\n");

  PoseData test_pose;
  create_sample_pose(&test_pose, 0.0f, 0.0f, 0.0f);

  result = segment_record_pose(&test_pose, "standing", "test_workout.json");
  if (result == SEGMENT_OK) {
    printf("✅ 포즈 기록 성공\n");
  } else {
    printf("❌ 포즈 기록 실패: %d\n", result);
  }

  // 4. 워크아웃 완성 테스트
  printf("4. 워크아웃 완성 테스트...\n");

  result = segment_finalize_workout_json("test_squat", "test_workout.json");
  if (result == SEGMENT_OK) {
    printf("✅ 워크아웃 완성 성공\n");
  } else {
    printf("❌ 워크아웃 완성 실패: %d\n", result);
  }

  printf("\n4. 정리 중...\n");
  segment_api_cleanup();
  printf("✅ 정리 완료\n");

  printf("\n=== 예제 완료 ===\n");
  return 0;
}
