#include <bits/stdc++.h>
using namespace std;

double stepTimes[4][2003];
int numSteps[4];
const double dt = 0.00000001;
void generateSCurveStepTimes(double *stepTimes, int& numSteps, int pulse_x, double T_mid)
{
    double v_max = 2 * pulse_x / T_mid;
    double t = 0.0, lastT = 0;
    double lastStepPos = 0.0, pos;
    numSteps = 0;

    // 当 t 超过加速时间或步数达到上限时停止计算
    while (numSteps < pulse_x)
    {
        // 计算当前位移 x(t)
        // 使用 smoothstep 模型： x(t) = v_max * (t^3/T_accel^2 - t^4/(2*T_accel^3))
        pos = v_max * ((t * t * t) / (T_mid * T_mid) - (t * t * t * t) / (2 * T_mid * T_mid * T_mid));

        // 当累计位移跨过下一个整数（即步数）时，记录该时刻
        if (floor(pos) > lastStepPos) {
            stepTimes[numSteps++] = ((t-lastT) * 1000000.0); // 转换为微秒
            lastStepPos = floor(pos);
            lastT = t;
        }
        t += dt;
    }
}

int main()
{
    
    double sum = 0;
    generateSCurveStepTimes(stepTimes[0], numSteps[0], 50, 0.1 / 2);
    cout << numSteps[0] << endl;
    for (int i = 0; i < numSteps[0]; i++) {
        sum += stepTimes[0][i];
        cout << stepTimes[0][i] << " ";
        if (i % 10 == 9)
            cout << endl;
    }
    printf("total time = %f\n", sum);
    return 0;
}