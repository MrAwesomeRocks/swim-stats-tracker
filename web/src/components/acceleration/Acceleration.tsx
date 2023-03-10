import { MpuDataContext } from "@/providers";
import { useContext } from "react";

import { LineChart } from "..";

interface AccelerationProps {}

// eslint-disable-next-line no-empty-pattern
export function Acceleration({}: AccelerationProps) {
    const { data } = useContext(MpuDataContext);

    const latestData = data.at(-1);
    const [x, y, z] = (latestData && latestData.accel) || [0, 0, 0];

    return (
        <div>
            <p>
                Acceleration: {x.toFixed(2)}, {y.toFixed(2)}, {z.toFixed(2)}{" "}
                (m/s<sup>2</sup>)
            </p>
            <LineChart
                title="Acceleration (w/o gravity)"
                height={400}
                xVals={data.map((e) => e.time)}
                yVals={data.reduce<number[][]>(
                    (acc, cur) => {
                        acc[0].push(cur.accel[0]);
                        acc[1].push(cur.accel[1]);
                        acc[2].push(cur.accel[2]);
                        return acc;
                    },
                    [[], [], []],
                )}
                series={[
                    {
                        label: "X",
                        stroke: "red",
                    },
                    {
                        label: "Y",
                        stroke: "blue",
                    },
                    {
                        label: "Z",
                        stroke: "green",
                    },
                ]}
                scales={{
                    x: {
                        time: false,
                    },
                }}
            />
        </div>
    );
}
