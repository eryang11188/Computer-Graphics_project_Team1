       //자동차 코드
   {
       float baseY = overlayY + WC::DRIVE_THK;
       float carLift = 0.55f;
       glm::vec3 carC(gateCenterX, baseY + carLift, driveCenterZ - 2.0f);

       glm::vec3 carGreen(0.22f, 0.72f, 0.30f);
       glm::vec3 carGreenDark(0.14f, 0.52f, 0.22f);
       glm::vec3 tire(0.07f, 0.07f, 0.07f);
       glm::vec3 rim(0.75f, 0.75f, 0.75f);
       glm::vec3 head(1.00f, 0.95f, 0.75f);
       glm::vec3 tail(0.88f, 0.15f, 0.12f);

       float bodyW = 2.80f;
       float bodyL = 5.30f;
       float bodyH = 0.85f;

       float cabinW = 2.35f;
       float cabinL = 2.70f;
       float cabinH = 0.95f;

       float wheelR = 0.58f;
       float wheelThk = 0.28f;

       // 1) 하부 차체
       AddBottom(glm::vec3(carC.x, carC.y, carC.z), glm::vec3(0),
           glm::vec3(bodyW, bodyH, bodyL), carGreen);

       // 2) 보닛
       AddBottom(glm::vec3(carC.x, carC.y + bodyH * 0.62f, carC.z + 1.40f), glm::vec3(0),
           glm::vec3(bodyW * 0.92f, 0.34f, 1.60f), carGreen);

       // 3) 범퍼(앞/뒤)
       AddBottom(glm::vec3(carC.x, carC.y + 0.06f, carC.z + bodyL * 0.5f - 0.12f), glm::vec3(0),
           glm::vec3(bodyW * 0.94f, 0.33f, 0.24f), carGreenDark);
       AddBottom(glm::vec3(carC.x, carC.y + 0.06f, carC.z - bodyL * 0.5f + 0.12f), glm::vec3(0),
           glm::vec3(bodyW * 0.94f, 0.33f, 0.24f), carGreenDark);

       // 4) 캐빈
       float cabinY = carC.y + bodyH;
       glm::vec3 cabinC(carC.x, cabinY, carC.z - 0.35f);
       AddBottom(glm::vec3(cabinC.x, cabinC.y, cabinC.z), glm::vec3(0),
           glm::vec3(cabinW, cabinH, cabinL), carGreen);

       // ----------------------------------------------------------
       // 옆 유리(좌/우) + 앞/뒤 좌석 경계선(B필러)
       // ----------------------------------------------------------
       float sideGlassT = 0.06f;             // X(좌우) 두께
       float sideWinH = cabinH * 0.65f;      // Y 높이
       float sideWinL = cabinL * 0.95f;      // Z(앞뒤) 길이

       float sideEps = 0.01f;
       float xL_glass = cabinC.x - (cabinW * 0.5f + sideEps);
       float xR_glass = cabinC.x + (cabinW * 0.5f + sideEps);

       float sideY = cabinC.y + cabinH * 0.62f;
       float sideZ = cabinC.z;

       glm::vec3 sideGlassCol(0.55f, 0.78f, 0.98f);

       AddCenter(glm::vec3(xL_glass, sideY, sideZ),
           glm::vec3(0.0f),
           glm::vec3(sideGlassT, sideWinH, sideWinL),
           sideGlassCol);

       AddCenter(glm::vec3(xR_glass, sideY, sideZ),
           glm::vec3(0.0f),
           glm::vec3(sideGlassT, sideWinH, sideWinL),
           sideGlassCol);

       // 앞좌석/뒷좌석 경계선(B필러) 추가 (좌/우)
       {
           glm::vec3 dividerCol = carGreenDark * 0.85f;

           float divT = sideGlassT + 0.02f;
           float divW = 0.10f;

           float splitZ = cabinC.z + cabinL * 0.10f;

           
           float pillarBaseY = carC.y - 0.01f;

          
           float pillarTopY = (cabinC.y + cabinH * 0.88f);

           float divH = pillarTopY - pillarBaseY;
           if (divH < 0.05f) divH = 0.05f;

           float divEps = 0.012f;
           float xLL = cabinC.x - (cabinW * 0.5f + divEps);
           float xRR = cabinC.x + (cabinW * 0.5f + divEps);

           AddBottom(glm::vec3(xLL, pillarBaseY, splitZ),
               glm::vec3(0.0f),
               glm::vec3(divT, divH, divW),
               dividerCol);

           AddBottom(glm::vec3(xRR, pillarBaseY, splitZ),
               glm::vec3(0.0f),
               glm::vec3(divT, divH, divW),
               dividerCol);
       }


       // ----------------------------------------------------------
       // 앞 유리(유지)
       // ----------------------------------------------------------
       {
           glm::vec3 glassCol(0.55f, 0.78f, 0.98f);

           float glassT = 0.10f;
           float winW = cabinW * 0.78f;
           float winH = cabinH * 0.90f;

           float y = cabinC.y + cabinH * 0.52f;
           float z = cabinC.z + cabinL * 0.5f + 0.04f;

           glm::vec3 rot(glm::radians(-12.0f), 0.0f, 0.0f);

           AddCenter(glm::vec3(cabinC.x, y, z),
               rot,
               glm::vec3(winW, winH, glassT),
               glassCol);
       }

       // ----------------------------------------------------------
       // 와이퍼
       // ----------------------------------------------------------
       {
           glm::vec3 wiperCol(0.06f, 0.06f, 0.06f);

           float glassT = 0.10f;
           float winW = cabinW * 0.78f;
           float winH = cabinH * 0.90f;

           float y = cabinC.y + cabinH * 0.52f;
           float z = cabinC.z + cabinL * 0.5f + 0.04f;

           glm::vec3 wsRot(glm::radians(-12.0f), 0.0f, 0.0f);

           glm::vec3 base = glm::vec3(cabinC.x, y - winH * 0.42f, z + 0.15f);

           float bladeT = 0.05f;
           float bladeH = 0.04f;
           float bladeL = winW * 0.42f;

           float armT = 0.04f;
           float armH = 0.04f;
           float armL = winW * 0.18f;

           {
               float xOff = -winW * 0.18f;
               glm::vec3 p = base + glm::vec3(xOff, 0.00f, 0.00f);

               AddCenter(p + glm::vec3(-armL * 0.20f, 0.01f, 0.00f),
                   wsRot + glm::vec3(0.0f, 0.0f, glm::radians(18.0f)),
                   glm::vec3(armL, armH, armT),
                   wiperCol);

               AddCenter(p,
                   wsRot + glm::vec3(0.0f, 0.0f, glm::radians(18.0f)),
                   glm::vec3(bladeL, bladeH, bladeT),
                   wiperCol);
           }

           {
               float xOff = +winW * 0.18f;
               glm::vec3 p = base + glm::vec3(xOff, 0.00f, 0.00f);

               AddCenter(p + glm::vec3(+armL * 0.20f, 0.01f, 0.00f),
                   wsRot + glm::vec3(0.0f, 0.0f, glm::radians(-18.0f)),
                   glm::vec3(armL, armH, armT),
                   wiperCol);

               AddCenter(p,
                   wsRot + glm::vec3(0.0f, 0.0f, glm::radians(-18.0f)),
                   glm::vec3(bladeL, bladeH, bladeT),
                   wiperCol);
           }
       }

       // ----------------------------------------------------------
       // 사이드미러
       // ----------------------------------------------------------
       {
           glm::vec3 mirrorBody(0.10f, 0.10f, 0.10f);
           glm::vec3 mirrorGlass(0.70f, 0.85f, 0.95f);

           float y = cabinC.y + cabinH * 0.58f;
           float z = carC.z + bodyL * 0.5f - 1.60f;

           float out = 0.18f;

           float armLen = 0.22f;
           float armThk = 0.06f;
           float armH = 0.08f;

           float housW = 0.30f;
           float housH = 0.20f;
           float housD = 0.16f;

           float glassInset = 0.01f;

           // 왼쪽(-X)
           {
               float xSide = cabinC.x - (cabinW * 0.5f + out);

               AddCenter(glm::vec3(xSide + armLen * 0.45f, y, z),
                   glm::vec3(0.0f),
                   glm::vec3(armLen, armH, armThk),
                   mirrorBody);

               AddCenter(glm::vec3(xSide, y, z),
                   glm::vec3(0.0f),
                   glm::vec3(housW, housH, housD),
                   mirrorBody);

               //  뒤쪽(-Z) 면에 붙여서 "뒤에서만" 보이게
               AddCenter(glm::vec3(xSide - housW * 0.10f, y, z - housD * 0.5f - glassInset),
                   glm::vec3(0.0f),
                   glm::vec3(housW * 0.78f, housH * 0.78f, 0.03f),
                   mirrorGlass);
           }

           // 오른쪽(+X)
           {
               float xSide = cabinC.x + (cabinW * 0.5f + out);

               AddCenter(glm::vec3(xSide - armLen * 0.45f, y, z),
                   glm::vec3(0.0f),
                   glm::vec3(armLen, armH, armThk),
                   mirrorBody);

               AddCenter(glm::vec3(xSide, y, z),
                   glm::vec3(0.0f),
                   glm::vec3(housW, housH, housD),
                   mirrorBody);

               AddCenter(glm::vec3(xSide + housW * 0.10f, y, z - housD * 0.5f - glassInset),
                   glm::vec3(0.0f),
                   glm::vec3(housW * 0.78f, housH * 0.78f, 0.03f),
                   mirrorGlass);
           }
       }

       // ----------------------------------------------------------
       // DOORS 
       // ----------------------------------------------------------
       {
           glm::vec3 doorLineCol = carGreenDark * 0.95f;
           glm::vec3 handleCol = glm::vec3(0.10f, 0.10f, 0.10f);

           float doorZ = carC.z + 0.10f;
           float doorY = carC.y + bodyH * 0.55f;

           float doorH = bodyH * 0.55f;
           float doorL = bodyL * 0.40f;
           float panelT = 0.03f;

           // 옆면 위치
           float eps = 0.005f;
           float xL = carC.x - (bodyW * 0.5f + panelT * 0.5f + eps);
           float xR = carC.x + (bodyW * 0.5f + panelT * 0.5f + eps);

           // 문 패널
           AddCenter(glm::vec3(xL, doorY, doorZ),
               glm::vec3(0.0f),
               glm::vec3(panelT, doorH, doorL),
               doorLineCol);

           AddCenter(glm::vec3(xR, doorY, doorZ),
               glm::vec3(0.0f),
               glm::vec3(panelT, doorH, doorL),
               doorLineCol);

           // 손잡이 (앞/뒤 2개씩) + 위로
           float handleW = 0.18f;
           float handleH = 0.05f;
           float handleT = 0.05f;

           float handleY = doorY + doorH * 0.28f;  //  위로 올림

           float handleZ_F = doorZ + doorL * 0.18f; // 앞쪽 손잡이
           float handleZ_B = doorZ - doorL * 0.18f; // 뒤쪽 손잡이

           // 왼쪽 2개
           AddCenter(glm::vec3(xL - handleT * 0.5f, handleY, handleZ_F),
               glm::vec3(0.0f),
               glm::vec3(handleT, handleH, handleW),
               handleCol);
           AddCenter(glm::vec3(xL - handleT * 0.5f, handleY, handleZ_B),
               glm::vec3(0.0f),
               glm::vec3(handleT, handleH, handleW),
               handleCol);

           // 오른쪽 2개
           AddCenter(glm::vec3(xR + handleT * 0.5f, handleY, handleZ_F),
               glm::vec3(0.0f),
               glm::vec3(handleT, handleH, handleW),
               handleCol);
           AddCenter(glm::vec3(xR + handleT * 0.5f, handleY, handleZ_B),
               glm::vec3(0.0f),
               glm::vec3(handleT, handleH, handleW),
               handleCol);
       }

       // ----------------------------------------------------------
       // 바퀴(링)
       // ----------------------------------------------------------
       auto AddWheelRing = [&](glm::vec3 wheelC, float radius, float thickness, glm::vec3 colTire, glm::vec3 colRim) {
           const int N = 24;
           for (int i = 0; i < N; ++i) {
               float a = (float)i / (float)N * 6.2831853f;
               float z = std::cos(a) * radius;
               float y = std::sin(a) * radius;
               glm::vec3 p = wheelC + glm::vec3(0.0f, y, z);

               AddCenter(p, glm::vec3(0.0f),
                   glm::vec3(thickness, radius * 0.14f, radius * 0.14f),
                   colTire);
           }
           AddCenter(wheelC, glm::vec3(0.0f),
               glm::vec3(thickness * 0.75f, radius * 0.62f, radius * 0.62f),
               colRim);
           };

       float wheelY = baseY + wheelR;
       float frontZ = carC.z + bodyL * 0.5f - 1.10f;
       float backZ = carC.z - bodyL * 0.5f + 1.10f;

       float wheelX = bodyW * 0.5f - wheelThk * 0.35f;

       glm::vec3 wFL(carC.x - wheelX, wheelY, frontZ);
       glm::vec3 wFR(carC.x + wheelX, wheelY, frontZ);
       glm::vec3 wBL(carC.x - wheelX, wheelY, backZ);
       glm::vec3 wBR(carC.x + wheelX, wheelY, backZ);

       AddWheelRing(wFL, wheelR, wheelThk, tire, rim);
       AddWheelRing(wFR, wheelR, wheelThk, tire, rim);
       AddWheelRing(wBL, wheelR, wheelThk, tire, rim);
       AddWheelRing(wBR, wheelR, wheelThk, tire, rim);

       // ----------------------------------------------------------
       // 라이트
       // ----------------------------------------------------------
       float lightZOut = 0.18f;

       float headY = carC.y + 0.48f;
       float headZ = carC.z + bodyL * 0.5f + lightZOut;
       AddBottom(glm::vec3(carC.x - 0.90f, headY, headZ),
           glm::vec3(0), glm::vec3(0.55f, 0.32f, 0.18f), head);
       AddBottom(glm::vec3(carC.x + 0.90f, headY, headZ),
           glm::vec3(0), glm::vec3(0.55f, 0.32f, 0.18f), head);

       float tailY = carC.y + 0.48f;
       float tailZ = carC.z - bodyL * 0.5f - lightZOut;
       AddBottom(glm::vec3(carC.x - 0.90f, tailY, tailZ),
           glm::vec3(0), glm::vec3(0.55f, 0.32f, 0.18f), tail);
       AddBottom(glm::vec3(carC.x + 0.90f, tailY, tailZ),
           glm::vec3(0), glm::vec3(0.55f, 0.32f, 0.18f), tail);

       // 스커트
       AddBottom(glm::vec3(carC.x, carC.y - 0.08f, carC.z),
           glm::vec3(0),
           glm::vec3(bodyW * 0.95f, 0.12f, bodyL * 0.95f),
           carGreenDark);

       {
           glm::vec3 plateCol(0.92f, 0.92f, 0.92f);   // 번호판 바탕
           glm::vec3 borderCol(0.12f, 0.12f, 0.12f);  // 테두리/숫자

           // 크기
           float plateW = 1.10f;   // 좌우(X)
           float plateH = 0.32f;   // 높이(Y)
           float plateT = 0.05f;   // 두께(Z)

           // 위치(앞/뒤): 차체 중앙, 범퍼 위쪽에 살짝 붙이기
           float plateY = carC.y + 0.30f;
           float frontZ = carC.z + bodyL * 0.5f + 0.12f;  // 앞쪽에 살짝 돌출
           float backZ = carC.z - bodyL * 0.5f - 0.12f;  // 뒤쪽에 살짝 돌출

           // 번호판 판(앞/뒤)
           AddCenter(glm::vec3(carC.x, plateY, frontZ), glm::vec3(0.0f),
               glm::vec3(plateW, plateH, plateT), plateCol);

           AddCenter(glm::vec3(carC.x, plateY, backZ), glm::vec3(0.0f),
               glm::vec3(plateW, plateH, plateT), plateCol);

           // 테두리(얇은 프레임 느낌) - 선택(원하면 삭제 가능)
           float b = 0.03f;
           AddCenter(glm::vec3(carC.x, plateY + plateH * 0.5f - b * 0.5f, frontZ + plateT * 0.6f), glm::vec3(0.0f),
               glm::vec3(plateW, b, b), borderCol);
           AddCenter(glm::vec3(carC.x, plateY - plateH * 0.5f + b * 0.5f, frontZ + plateT * 0.6f), glm::vec3(0.0f),
               glm::vec3(plateW, b, b), borderCol);

           AddCenter(glm::vec3(carC.x, plateY + plateH * 0.5f - b * 0.5f, backZ - plateT * 0.6f), glm::vec3(0.0f),
               glm::vec3(plateW, b, b), borderCol);
           AddCenter(glm::vec3(carC.x, plateY - plateH * 0.5f + b * 0.5f, backZ - plateT * 0.6f), glm::vec3(0.0f),
               glm::vec3(plateW, b, b), borderCol);

           // "1111" 숫자: 얇은 세로 막대 4개(앞/뒤)
           float digitW = 0.06f;        // 막대 두께(X)
           float digitH = plateH * 0.70f;
           float digitT = 0.03f;        // 판 위로 살짝 올라오게(Z)
           float spacing = 0.16f;       // 숫자 간격
           float startX = carC.x - spacing * 1.5f;

           for (int i = 0; i < 4; ++i) {
               float x = startX + spacing * i;

               // 앞판 숫자
               AddCenter(glm::vec3(x, plateY, frontZ + plateT * 0.55f), glm::vec3(0.0f),
                   glm::vec3(digitW, digitH, digitT), borderCol);

               // 뒷판 숫자(뒤쪽에서 보이도록 반대 방향으로)
               AddCenter(glm::vec3(x, plateY, backZ - plateT * 0.55f), glm::vec3(0.0f),
                   glm::vec3(digitW, digitH, digitT), borderCol);
           }
       }
   }