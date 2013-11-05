//
//  main.c
//  appdeploy
//
//  Created by Chip Snyder on 10/17/13.
//  Copyright (c) 2013 Education. All rights reserved.
//

#include "mobiledevice.h"
#include <stdlib.h>
#include <string.h>

#define ASSERT_OR_EXIT(_cnd_, ...) do { if(!(_cnd_)) { fprintf(stderr, __VA_ARGS__); unregister_device_notification(1); } } while (0)

// Object Structures
enum MobileDeviceCommandType
{
    GetUDID,
    InstallApp,
    UninstallApp,
    ListApps
};

struct
{
    struct am_device_notification *notification;
    enum MobileDeviceCommandType type;
    const char *app_path;
    const char *bundle_id;
    int print_paths;
    uint16_t src_port;
    uint16_t dst_port;
} command;

void print_usage()
{
    printf("\nUsage: appdeploy <command> [<options>]\n");
    printf("<path_to_app>                  : Local Path to .app file. ex /Users/me/Documents/CumberTest.app \n");
    printf("<bundle_id>                    : Bundle Identification of application example. com.apple.Music \n\n");
    printf("Commands:\n");
    printf("  get_udid                     : Display UDID of connected device (will only show the first device discovered) \n");
    printf("  get_bundle_id <path_to_app>  : Display bundle identifier of app \n");
    printf("  install <path_to_app>        : Install app to device\n");
    printf("  uninstall <bundle_id>        : Uninstall app by bundle id\n");
    printf("  tunnel <from_port> <to_port> : Forward TCP connections to connected device\n");
    printf("  list_apps [-p]               : Lists all installed apps on device\n");
    printf("             -p                : Include installation paths\n");
    printf("\n");
    
}

// Unregister notifications
void unregister_device_notification(int status)
{
    AMDeviceNotificationUnsubscribe(command.notification);
    exit(status);
}

// Helpers
char *create_cstr_from_cfstring(CFStringRef cfstring)
{
    CFIndex str_length = CFStringGetLength(cfstring);
    
    if (str_length < 0)
    {
        return NULL;
    }
    
    CFIndex buf_length = CFStringGetMaximumSizeForEncoding(str_length, kCFStringEncodingUTF8) + 1;
    
    if (buf_length < 1)
    {
        return NULL;
    }
    
    char *cstr = (char *)malloc(buf_length);
    
    if (cstr == NULL)
    {
        return NULL;
    }
    
    if (CFStringGetCString(cfstring, cstr, buf_length, kCFStringEncodingUTF8))
    {
        return cstr;
    }
    
    return NULL;
}

CFURLRef get_absolute_file_url(const char *file_path)
{
    CFStringRef path = CFStringCreateWithCString(NULL, file_path, kCFStringEncodingUTF8);
    CFURLRef relative_url = CFURLCreateWithFileSystemPath(NULL, path, kCFURLPOSIXPathStyle, false);
    CFURLRef url = CFURLCopyAbsoluteURL(relative_url);
    
    CFRelease(path);
    CFRelease(relative_url);
    return url;
}

void connect_to_device(struct am_device *device)
{
    AMDeviceConnect(device);
    ASSERT_OR_EXIT(AMDeviceIsPaired(device), "Error attempting to connect to device: AMDeviceIsPaired failed\n");
    ASSERT_OR_EXIT(!AMDeviceValidatePairing(device), "Error attempting to connect to device: AMDeviceValidatePairing failed\n");
    ASSERT_OR_EXIT(!AMDeviceStartSession(device), "Error attempting to connect to device: AMDeviceStartSession failed\n");
}

// Get UDID
void get_udid(struct am_device *device)
{
    char *udid = create_cstr_from_cfstring(AMDeviceCopyDeviceIdentifier(device));
    
    if (udid == NULL)
    {
        unregister_device_notification(1);
    }
    
    // print UDID to console
    printf("%s\n", udid);
    
    free(udid);
    unregister_device_notification(0);
}

// Get Bundle ID
CFStringRef read_plist_for_app_path(const char *app_path)
{
    CFURLRef app_url = get_absolute_file_url(app_path);
    
    if (app_url == NULL)
    {
        return NULL;
    }
    
    CFURLRef url = CFURLCreateCopyAppendingPathComponent(NULL, app_url, CFSTR("Info.plist"), false);
    CFRelease(app_url);
    
    if (url == NULL)
    {
        return NULL;
    }
    
    CFReadStreamRef stream = CFReadStreamCreateWithFile(NULL, url);
    CFRelease(url);
    
    if (stream == NULL)
    {
        return NULL;
    }
    
    CFPropertyListRef plist = NULL;
    
    if (CFReadStreamOpen(stream) == TRUE)
    {
        plist = CFPropertyListCreateWithStream(NULL, stream, 0, kCFPropertyListImmutable, NULL, NULL);
    }
    
    CFReadStreamClose(stream);
    CFRelease(stream);
    
    if (plist == NULL)
    {
        return NULL;
    }
    
    const void *value = CFDictionaryGetValue(plist, CFSTR("CFBundleIdentifier"));
    CFStringRef bundle_id = NULL;
    
    if (value != NULL)
    {
        bundle_id = CFRetain(value);
    }
    
    CFRelease(plist);
    
    return bundle_id;
}

void get_bundle_id(const char *app_path)
{
    CFStringRef bundle_id = read_plist_for_app_path(app_path);
    
    if (bundle_id == NULL)
    {
        exit(1);
    }
    
    char *_bundle_id = create_cstr_from_cfstring(bundle_id);
    CFRelease(bundle_id);
    
    if (_bundle_id == NULL)
    {
        exit(1);
    }
    
    printf("%s\n", _bundle_id);
    free(_bundle_id);
    exit(0);
}

// Install App
void install_app(struct am_device *device)
{
    connect_to_device(device);
    
    CFURLRef local_app_url = get_absolute_file_url(command.app_path);
    CFStringRef keys[] = { CFSTR("PackageType") }, values[] = { CFSTR("Developer") };
    CFDictionaryRef options = CFDictionaryCreate(NULL, (const void **)&keys, (const void **)&values, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    // copy .app to device
    ASSERT_OR_EXIT(!AMDeviceSecureTransferPath(0, device, local_app_url, options, NULL, 0), "Error attempting to install app: AMDeviceSecureTransferPath failed\n");
    
    // install package on device
    ASSERT_OR_EXIT(!AMDeviceSecureInstallApplication(0, device, local_app_url, options, NULL, 0), "Error attempting to install app: AMDeviceSecureInstallApplication failed\n");
    
    CFRelease(options);
    CFRelease(local_app_url);
    
    printf("%s successfully installed.\n", command.app_path);
    unregister_device_notification(0);
}

// Uninstall App
static void print_installed_app(const void *key, const void *value, void *context)
{
    if ((key == NULL) || (value == NULL))
    {
        return;
    }
    
    char *bundle_id = create_cstr_from_cfstring((CFStringRef)key);
    
    if (bundle_id == NULL)
    {
        return;
    }
    
    if (command.print_paths)
    {
        char *path = NULL;
        CFStringRef path_value;
        path_value = CFDictionaryGetValue((CFDictionaryRef)value, CFSTR("Path"));
        
        if (path_value != NULL)
        {
            path = create_cstr_from_cfstring(path_value);
        }
        
        printf("%s\n\tPath: %s\n", bundle_id, (path == NULL) ? "" : path);
        
        if (path != NULL)
        {
            free(path);
        }
    }
    else
    {
        printf("%s\n", bundle_id);
    }
    
    free(bundle_id);
}

void uninstall_app(struct am_device *device)
{
    connect_to_device(device);
    CFStringRef bundle_id = CFStringCreateWithCString(NULL, command.bundle_id, kCFStringEncodingUTF8);
    
    // uninstall package from device
    ASSERT_OR_EXIT(!AMDeviceSecureUninstallApplication(0, device, bundle_id, 0, NULL, 0), "Error attempting to uninstall app: AMDeviceSecureUninstallApplication failed\n");
    
    CFRelease(bundle_id);
    
    printf("%s successfully uninstalled.\n", command.bundle_id);
    unregister_device_notification(0);
}

// List Apps
void list_apps(struct am_device *device)
{
    connect_to_device(device);
    
    CFDictionaryRef apps;
    ASSERT_OR_EXIT(!AMDeviceLookupApplications(device, 0, &apps), "Error attempting to list installed apps: AMDeviceLookupApplications failed\n");
    
    CFDictionaryApplyFunction(apps, print_installed_app, NULL);
    CFRelease(apps);
    
    unregister_device_notification(0);
}


// Device Connected
void on_device_connected(struct am_device *device)
{
    if (command.type == GetUDID)
    {
        get_udid(device);
    }
    else if (command.type == InstallApp)
    {
        install_app(device);
    }
    else if (command.type == UninstallApp)
    {
        uninstall_app(device);
    }
    else if (command.type == ListApps)
    {
        list_apps(device);
    }
}

void on_device_notification(struct am_device_notification_callback_info *info, int cookie)
{
    switch (info->msg)
    {
        case ADNCI_MSG_CONNECTED:
            on_device_connected(info->dev);
            break;
            
        default:
            break;
    }
}

void register_device_notification()
{
    AMDeviceNotificationSubscribe(&on_device_notification, 0, 0, 0, &command.notification);
    CFRunLoopRun();
}

// Main Run Loop
int main(int argc, const char * argv[])
{
    if ((argc == 2) && (strcmp(argv[1], "get_udid") == 0))
    {
        command.type = GetUDID;
    }
    else if ((argc ==3) && (strcmp(argv[1], "get_bundle_id") == 0))
    {
        get_bundle_id(argv[2]);
    }
    else if ((argc == 3) && (strcmp(argv[1], "install") == 0))
    {
        command.type = InstallApp;
        command.app_path = argv[2];
    }
    else if((argc == 3) && (strcmp(argv[1], "uninstall") == 0))
    {
        command.type = UninstallApp;
        command.bundle_id = argv[2];
    }
    else if((argc >= 2) && (strcmp(argv[1], "list_apps") == 0))
    {
        command.type = ListApps;
        
        if ((argc == 3) && (strcmp(argv[2], "-p") == 0))
        {
            command.print_paths = 1;
        }
        else
        {
            command.print_paths = 0;
        }
        
    }
    else
    {
        print_usage();
        exit(1);
    }
    
    register_device_notification();
    return 1;
}

