# XP-DLP: Lightweight Data Loss Prevention Tool for Windows  

XP-DLP is a modern Data Loss Prevention (DLP) tool designed for Windows XP and newer versions. It provides robust whitelist enforcement, flexible state management, and seamless service integration to protect your network from unauthorized connections.  

---

## Key Features  
- **Whitelist Enforcement**: Block all unauthorized connections.  
- **Time-Limited Access**: Allow temporary connections for a specified duration.  
- **Flexible Modes**: Operates in Blocking, Learning, and Unblocking states.  
- **Automatic Reversion**: Automatically switches back to Blocking mode after 5 minutes.  
- **Service Integration**: Runs seamlessly as a Windows service.  

---

## Tool States  

XP-DLP operates in **three modes**, providing flexibility based on your security needs:  

| **State**      | **Description**                                                                 | **Registry Value** |
|-----------------|---------------------------------------------------------------------------------|--------------------|
| **Blocking**    | Denies all connections not listed in the whitelist (Default mode).             | `0`                |
| **Learning**    | Allows all connections and automatically adds them to the whitelist.           | `1`                |
| **Unblocking**  | Permits all connections without restrictions.                                  | `2`                |

### State Management  
To change the mode, follow these steps:  
1. Open the registry editor (`regedit`).  
2. Navigate to: HKLM\SOFTWARE\Microsoft\Hardening\
   3. Update the `state` value:  
- `0` - Blocking  
- `1` - Learning  
- `2` - Unblocking  

### Automatic Reversion  
State changes last for **5 minutes**, after which XP-DLP automatically reverts to **Blocking (0)** to ensure secure operation.  

---

## Compilation Instructions  

To compile XP-DLP, ensure the following environment setup:  
- **C++ Standard**: C++17  
- **Platform Toolset**: v141  

### Steps:  
1. Open the project in Visual Studio.  
2. Set the Platform Toolset to `v141` in the project settings.  
3. Build the solution to generate the executable.  

---

## Usage Instructions  

### Installation  
1. Open a Command Prompt with administrator privileges.  
2. Run the following command to install the service:  
```cmd  
XP_DLP.exe install
```
3. Start the service
```cmd
sc start XP_DLP
```
