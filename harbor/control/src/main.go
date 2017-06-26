package main

import (
    "os"
    "fmt"
    "log"
    "net"
    "time"
    "bufio"
    "bytes"
    "strconv"
    "strings"
    "net/http"
    "encoding/json"

    "golang.org/x/net/context"
    "google.golang.org/grpc"
    pb "./.rpc"
)

type FaceBox struct {
    Left float32   `json:"left"`
    Top float32    `json:"top"`
    Width float32  `json:"width"`
    Height float32 `json:"height"`
}

type FaceName struct {
    Name string     `json:"name"`
    Quality float32 `json:"quality"`
}

type Face struct {
    Rect FaceBox        `json:"box"`
    Markers [][]float32 `json:"markers"`
    Names []FaceName    `json:"names"`
}

type NetDevice struct {
    Address string      `json:"address"`
    Manufacturer string `json:"man"`
    Signal int          `json:"signal"`
    TimeStamp int64     `json:"timestamp"`
}

const TransitionSecondsIn = 2
const TransitionSecondsOut = 5

type FaceState struct {
    // 0 - not seen for a long time
    // 1 - seen in <N seconds, update LastSeen
    // 2 - seen in >N seconds, do the greeting, update LastSeen
    //     if Now - LastSeen, then go to state 0
    State int

    FirstSeen time.Time // first time when face is seen
    LastSeen time.Time  // last time when face is seen

    Count int // incremented when state goes 1 -> 2

    Updated bool
}

type Status struct {
    Faces []Face
    Devices map[string]NetDevice

    State map[string]*FaceState

    Frame int
    OldImage string
    NewImage string
}

var status = Status{}

func onStatic(w http.ResponseWriter, r *http.Request) {
    if strings.HasPrefix(r.URL.Path, "/images/") {
        http.ServeFile(w, r, "/tmp/" + strings.TrimPrefix(r.URL.Path, "/images/"))
        return
    }
    http.ServeFile(w, r, "static/index.html")
}

func onDevice(w http.ResponseWriter, r *http.Request) {
    var dev NetDevice
    decoder := json.NewDecoder(r.Body)
    err := decoder.Decode(&dev)
    if err != nil {
        fmt.Printf("cannot parse json: %v\n", err)
        w.WriteHeader(http.StatusBadRequest)
        return
    }

    status.Devices[dev.Address] = dev
}

func onStatus(w http.ResponseWriter, r *http.Request) {

    type Response struct {
        Faces []Face
        Devices []NetDevice
        Image string
        Greeting string
    }

    if len(status.NewImage) != 0 {
        status.OldImage = status.NewImage
        status.NewImage = ""

        old := fmt.Sprintf("/tmp/frame%04d.jpg", status.Frame)
        os.Remove(old)

        status.Frame++
    }

    var url string
    if len(status.OldImage) != 0 {
        url = "images/" + status.OldImage;
    }

    devices := make([]NetDevice, 0, len(status.Devices))
    for _, dev := range status.Devices {
        devices = append(devices, dev)
    }

    const Quality = 0.8

    for _, face := range status.Faces {
        if len(face.Names) > 0 && face.Names[0].Quality > Quality {
            name := face.Names[0].Name

            st, exists := status.State[name]
            if exists {

                switch st.State {
                case 0:
                    st.FirstSeen = time.Now()
                    st.State = 1
                case 1:
                    st.LastSeen = time.Now()
                    if time.Now().Sub(st.FirstSeen).Seconds() > TransitionSecondsIn {
                        st.State = 2
                        st.Count++
                    }
                case 2:
                    st.LastSeen = time.Now()
                }
                st.Updated = true

            } else {

                status.State[name] = &FaceState{0, time.Now(), time.Time{}, 0, true}
            }
        }
    }

    for _, st := range status.State {
        if st.Updated {
            st.Updated = false
            continue
        }

        if time.Now().Sub(st.LastSeen).Seconds() > TransitionSecondsOut {
            st.State = 0
        }
    }

    var greeting string

    for name, st := range status.State {
        if st.State == 2 {
            if st.Count == 1 {
                greeting += fmt.Sprintf("Welcome %s!\n", name)
            } else {
                greeting += fmt.Sprintf("Hello %s, good to have you back (%d)\n", name, st.Count)
            }
        }
    }

    response := Response { status.Faces, devices, url, greeting }

    data, err := json.Marshal(response)
    if err != nil {
        w.WriteHeader(http.StatusInternalServerError)
        return
    }

    w.Header()["Content-Type"] = []string{ "application/json" }
    w.Write(data)
}

func netscan() {

    var err error
    var conn net.Conn
    var reader *bufio.Reader

    for {
        if conn == nil {
            time.Sleep(100 * time.Millisecond)
            conn, err = net.Dial("tcp", os.Getenv("NETSCAN"))
            if err != nil {
                fmt.Printf("ERROR: cannot connect to kismet: %v\n", err)
                time.Sleep(3000 * time.Millisecond)
                continue
            }

            _, err = fmt.Fprintf(conn, "!0 ENABLE CLIENT *\n")
            if err != nil {
                fmt.Printf("ERROR: cannot send command to kismet: %v\n", err)
                conn.Close()
                conn = nil
                continue
            }

            reader = bufio.NewReader(conn)
            if reader == nil {
                fmt.Printf("ERROR: cannot create kismet socket reader: %v\n", err)
                conn.Close()
                conn = nil
                continue
            }
        }

        str, err := reader.ReadString('\n')
        if err != nil {
            fmt.Printf("ERROR: cannot read data from kismet: %v\n", err)
            conn.Close()
            conn = nil
            continue
        }

        // CLIENT bssid,mac,type,firsttime,lasttime,manuf,llcpackets,datapackets,cryptpackets,gpsfixed,minlat,minlon,minalt,minspd,maxlat,maxlon,maxalt,maxspd,agglat,agglon,aggalt,aggpoints,signal_dbm,noise_dbm,minsignal_dbm,minnoise_dbm,maxsignal_dbm,maxnoise_dbm,signal_rssi,noise_rssi,minsignal_rssi,minnoise_rssi,maxsignal_rssi,maxnoise_rssi,bestlat,bestlon,bestalt,atype,ip,gatewayip,datasize,maxseenrate,encodingset,carrierset,decrypted,channel,fragments,retries,newpackets,freqmhz,cdpdevice,cdpport,dot11d,dhcphost,dhcpvendor,datacryptset
        //   0      1    2   3       4         5      6        7           8          9         10       11     12    13      14     15     16     17      18     19     20    21      22         23         24         25           26            27            28           29         30          31             32            33              34         35      36      37     38  39     40         41       42          43           44       45       46        47      48       49        50       51       52     53      54        55         56

        if strings.HasPrefix(str, "*CLIENT: ") {
            fields := strings.Split(str, " ")

            ssid := fields[1]
            address := fields[2]

            if ssid == address {
                signal, _ := strconv.Atoi(fields[23])
                timestamp, _ := strconv.ParseInt(fields[5], 10, 64)
                man := fields[6]
                man = man[1:len(man)-1]

                dev := NetDevice { address, man, signal, timestamp }

                j, _ := json.Marshal(dev)

                r, err := http.Post("http://localhost:8080/device", "application/json", bytes.NewReader(j))
                if err != nil {
                    fmt.Printf("ERROR: cannot post device data: %v\n", err)
                } else {
                    if r.StatusCode != 200 {
                        fmt.Printf("ERROR: http errr: %v\n", r.Status)
                    }
                    r.Body.Close()
                }
            }
        }
    }
}

type server struct{}

func (s *server) PostData(ctx context.Context, in *pb.Data) (*pb.Empty, error) {

    faces := make([]Face, 0, len(in.Faces))

    for _, f := range in.Faces {

        face := Face{}

        face.Rect.Left = f.Box.Left
        face.Rect.Top = f.Box.Top
        face.Rect.Width = f.Box.Width
        face.Rect.Height = f.Box.Height

        face.Markers = make([][]float32, 0, len(f.Markers.Points))
        for _, p := range f.Markers.Points {
            face.Markers = append(face.Markers, []float32{ p.X, p.Y })
        }

        face.Names = make([]FaceName, 0, len(f.Names))
        for _, n := range f.Names {
            face.Names = append(face.Names, FaceName{n.Name, n.Quality})
        }

        faces = append(faces, face)
    }

    status.Faces = faces

    return &pb.Empty{}, nil
}

func (s *server) PostFrame(ctx context.Context, in *pb.Frame) (*pb.Empty, error) {

    status.NewImage = fmt.Sprintf("frame%04d.jpg", status.Frame+1)

    f, err := os.Create("/tmp/" + status.NewImage)
    if err != nil {
        fmt.Printf("Cannot create image: %v\n", err)
        return &pb.Empty{}, nil
    }
    defer f.Close()

    _, err = f.Write(in.Image)

    if err != nil {
        fmt.Printf("Cannot save image: %v\n", err)
        return &pb.Empty{}, nil
    }

    return &pb.Empty{}, nil
}

func rpc() {

    sock, err := net.Listen("tcp", ":50051")
    if err != nil {
        log.Fatalf("failed to listen: %v", err)
    }

    s := grpc.NewServer()
    pb.RegisterControlServerServer(s, &server{})

    if err := s.Serve(sock); err != nil {
        log.Fatalf("failed to serve: %v", err)
    }
}

func main() {
    status.Devices = make(map[string]NetDevice)
    status.State = make(map[string]*FaceState)

    go netscan()
    go rpc()

    http.HandleFunc("/device", onDevice)
    http.HandleFunc("/status", onStatus)
    http.HandleFunc("/", onStatic)

    http.ListenAndServe(":8080", nil)
}
