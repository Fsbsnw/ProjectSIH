# DialogueSubsystem 기반 대화 시스템

[← README](../README.md) · [조사·회의](Investigation_Meeting.md)

조사와 회의에서 같은 대화 실행기를 사용할 수 있도록 **대화의 진행·분기**와 **실제 게임플레이 판정**을 분리했습니다.

> [이미지 삽입: 대화 선택지와 Action 분기를 보여주는 간단한 흐름도]

## 데이터와 실행

- `UPFDialogueDefinition`은 Dialogue·Action·End 노드와 시작 노드를 데이터로 보관합니다.
- `UPFDialogueSubsystem`은 현재 대사와 선택지를 UI에 전달하고, Choice ID로 다음 노드를 선택합니다.
- 외부 처리가 필요한 Action 노드는 요청을 전달한 뒤 Result ID를 받아 분기합니다.
- 조사 단서 획득과 회의 단서 제출은 각각 담당 시스템이 판정하고 결과만 대화 실행기로 돌려줍니다.

`대화 노드 → 선택지 / Action 요청 → 외부 시스템 결과 → 다음 노드 또는 종료`

## 구현 시 고려한 점

노드 참조·ID 중복 등을 시작 전에 검사하고, 이미 지난 선택지나 오래된 Action 응답이 현재 대화를 변경하지 않도록 방어 경로를 마련했습니다. 현재 공개 코드는 대화 런타임과 테스트용 UI의 C++ 구조를 중심으로 하며 최종 콘텐츠·UI 구현을 완료한 것으로 소개하지 않습니다.

> [GIF 삽입: 대화 진행 → 선택지 → 조사 Action 결과에 따른 분기. 실제 구현된 테스트 화면 사용]

## 주요 코드

- [PFDialogueTypes.h](../Project_SIH/000_Core/001_Contracts/006_Dialogue/PFDialogueTypes.h)
- [PFDialogueDefinition.cpp](../Project_SIH/001_Data/000_Definitions/PFDialogueDefinition.cpp)
- [PFDialogueSubsystem.cpp](../Project_SIH/002_Systems/011_Dialogue/PFDialogueSubsystem.cpp)
- [PFMeetingSystem.cpp](../Project_SIH/002_Systems/001_Investigation/PFMeetingSystem.cpp)
