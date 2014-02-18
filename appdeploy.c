//
//  main.c
//  appdeploy
//
//  Created by Chip Snyder on 10/17/13.
//  Copyright (c) 2013 Education. All rights reserved.
//

#include "mobiledevice.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define ASSERT_OR_EXIT(_cnd_, ...) do { if(!(_cnd_)) { fprintf(stderr, __VA_ARGS__); unregister_device_notification(1); } } while (0)

// Object Structures
enum MobileDeviceCommandType
{
    GetUDID,
    InstallApp,
    UninstallApp,
    ListApps,
    ListFiles,
    RemoveFile,
    DownloadFile,
    UploadFile
};

struct
{
    struct am_device_notification *notification;
    enum MobileDeviceCommandType type;
    char *target;
    char *app_path;
    char *bundle_id;
    char *file_path;
    char *destination_path;
    int print_paths;
    uint16_t src_port;
    uint16_t dst_port;
} command;

void print_usage()
{
    printf("\nUsage: appdeploy <command> [<options>]\n");
    printf("    -p <path_to_app>\n");
    printf("        - Local Path to .app file. ex /Users/me/Documents/CumberTest.app \n\n");
    printf("    -b <bundle_id>\n");
    printf("        - Bundle Identification of application. ex com.apple.Music \n\n");
    printf("    -f <file_path>\n");
    printf("        - The path to the file on the device. ex /Documents/File.png \n\n");
    printf("    -dest <destination_path>\n");
    printf("        - The local path to store the downloaded file. ex /Users/me/File.png \n\n");
    printf("    -t <target_device>\n");
    printf("        - The device to target with the selected action. Will install to the first device found if not specified\n\n");
    printf("    -v (verbose)\n");
    printf("        - Enables the verbose output where available.\n\n");
    printf("Commands:\n");
    printf("    get_udid\n");
    printf("        - Display UDID of connected device (will only show the first device discovered) \n\n");
    printf("    get_bundle_id <path_to_app>\n");
    printf("        - Display bundle identifier of app \n\n");
    printf("    install -p <path_to_app> [-t <target_device>]\n");
    printf("        - Install app to device\n\n");
    printf("    uninstall -b <bundle_id> [-t <target_device>]\n");
    printf("        - Uninstall app by bundle id\n\n");
    printf("    remove_file -b <bundle_id> -f <file_path> [-t <target_device>]\n");
    printf("        - Deletes the specified file at the given path\n\n");
    printf("    download_file -b <bundle_id> -f <file_path> -dest <destination_path> [-t <target_device>]\n");
    printf("        - Deletes the specified file at the given path\n\n");
    printf("    upload_file -b <bundle_id> -f <file_path> -dest <destination_path> [-t <target_device>]\n");
    printf("        - Upload the specified file at the given path\n\n");
    printf("    list_files -b <bundle_id> [-v] [-t <target_device>]\n");
    printf("        - Lists all of the files in the sandbox for the specified app.\n");
    printf("        - Use the optional -v paramater to get also list all directories\n\n");
    printf("    list_apps [-v] [-t <target_device>]\n");
    printf("        - Lists all installed apps on device\n");
    printf("        - Use the optional -v paramater to include all application installation paths\n\n");
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
}

// Uninstall App
void uninstall_app(struct am_device *device)
{
    connect_to_device(device);
    CFStringRef bundle_id = CFStringCreateWithCString(NULL, command.bundle_id, kCFStringEncodingUTF8);
    
    // uninstall package from device
    ASSERT_OR_EXIT(!AMDeviceSecureUninstallApplication(0, device, bundle_id, 0, NULL, 0), "Error attempting to uninstall app: AMDeviceSecureUninstallApplication failed\n");
    
    CFRelease(bundle_id);
    
    printf("%s successfully uninstalled.\n", command.bundle_id);
}

// List Apps
void list_apps(struct am_device *device)
{
    connect_to_device(device);
    
    CFDictionaryRef apps;
    ASSERT_OR_EXIT(!AMDeviceLookupApplications(device, 0, &apps), "Error attempting to list installed apps: AMDeviceLookupApplications failed\n");
    
    CFDictionaryApplyFunction(apps, print_installed_app, NULL);
    CFRelease(apps);
}

// List Files
void read_files(struct afc_connection* fileConnection, char* dir)
{
    char *dir_ent;
    
    struct afc_dictionary* fileDictionary;
    AFCFileInfoOpen(fileConnection, dir, &fileDictionary);
    
    struct afc_directory* fileDirectory;
    afc_error_t err = AFCDirectoryOpen(fileConnection, dir, &fileDirectory);
    
    if (err != 0)
    {
        printf("%s\n", dir);
        return;
    }
    else if (command.print_paths)
    {
        printf("%s\n", dir);
    }
    
    while(true)
    {
        err = AFCDirectoryRead(fileConnection, fileDirectory, &dir_ent);
        
        if (!dir_ent)
        {
            break;
        }
        
        if (strcmp(dir_ent, ".") == 0 || strcmp(dir_ent, "..") == 0)
        {
            continue;
        }
        
        char* dir_joined = malloc(strlen(dir) + strlen(dir_ent) + 2);
        strcpy(dir_joined, dir);
        
        if (dir_joined[strlen(dir)-1] != '/')
        {
            strcat(dir_joined, "/");
        }
        
        strcat(dir_joined, dir_ent);
        read_files(fileConnection, dir_joined);
        free(dir_joined);
    }
    
    AFCDirectoryClose(fileConnection, fileDirectory);
}

service_conn_t start_file_service(struct am_device * device)
{
    connect_to_device(device);
    
    service_conn_t serviceConnection;
    
    CFStringRef cf_bundle_id = CFStringCreateWithCString(NULL, command.bundle_id, kCFStringEncodingASCII);
    if (AMDeviceStartHouseArrestService(device, cf_bundle_id, 0, &serviceConnection, 0) != 0)
    {
        printf("Unable to find bundle with id: %s\n", command.bundle_id);
        exit(1);
    }
    
    ASSERT_OR_EXIT(AMDeviceStopSession(device) == 0, "Error attempting to list files: AMDeviceStopSession failed\n");
    ASSERT_OR_EXIT(AMDeviceDisconnect(device) == 0, "Error attempting to list files: AMDeviceDisconnect failed\n");
    CFRelease(cf_bundle_id);
    
    return serviceConnection;
}


void list_files(struct am_device *device)
{
    service_conn_t serviceConnection = start_file_service(device);
    
    struct afc_connection* fileConnection;
    AFCConnectionOpen(serviceConnection, 0, &fileConnection);
    
    read_files(fileConnection, "/Documents");
}

//Remove File

void delete_file(struct am_device *device)
{
    service_conn_t serviceConnection = start_file_service(device);
    struct afc_connection* fileConnection;
    AFCConnectionOpen(serviceConnection, 0, &fileConnection);
    
    char *fileDir = command.file_path;
    
    ASSERT_OR_EXIT(AFCRemovePath(fileConnection, fileDir) == 0, "Error attempting to remove file: AFCRemovePath failed\n");
    ASSERT_OR_EXIT(AFCConnectionClose(fileConnection) == 0, "Error attempting to remove file: AFCConnectionClose failed\n");
    
    printf("%s successfully removed.\n", command.file_path);
}

//Downlaod File

void download_file(struct am_device *device)
{
    service_conn_t serviceConnection = start_file_service(device);
    struct afc_connection* fileConnection;
    AFCConnectionOpen(serviceConnection, 0, &fileConnection);
    
    char *fileDir = command.file_path;
    afc_file_ref file_ref;
    
    ASSERT_OR_EXIT(AFCFileRefOpen(fileConnection, fileDir, 2, &file_ref) == 0, "Error attempting to download file: AFCFileRefOpen failed\n");
    
    struct afc_dictionary* fileDictionary;
    ASSERT_OR_EXIT(AFCFileInfoOpen(fileConnection, fileDir, &fileDictionary) == 0, "Error attempting to download file: AFCFileInfoOpen failed\n");
    
    char *pValue, *pKey;
    unsigned int fsize = 0;
    
    AFCKeyValueRead(fileDictionary, &pKey, &pValue);
    
	while(pKey || pValue)
    {
		if (pKey == NULL || pValue == NULL)
        {
			break;
        }
		if (!strcmp(pKey, "st_size"))
        {
			fsize = (unsigned int)atoi(pValue);
			break;
		}
		AFCKeyValueRead(fileDictionary, &pKey, &pValue);
	}
	AFCKeyValueClose(fileDictionary);
    
    char* buf = malloc(fsize);
    
    ASSERT_OR_EXIT(AFCFileRefRead(fileConnection, file_ref, buf, &fsize) == 0, "Error attempting to download file: AFCFileRefRead failed\n");
    FILE* pFile = fopen(command.destination_path, "wb");
    
    if (pFile)
    {
        fwrite(buf, fsize, 1, pFile);
    }
    fclose(pFile);
    
    ASSERT_OR_EXIT(AFCFileRefClose(fileConnection, file_ref) == 0, "Error attempting to download file: AFCFileRefClose failed\n");
    ASSERT_OR_EXIT(AFCConnectionClose(fileConnection) == 0, "Error attempting to download file: AFCConnectionClose failed\n");
    
    printf("%s successfully downloaded to %s.\n", command.file_path, command.destination_path);
}

//Upload File

void upload_file(struct am_device *device)
{
    service_conn_t serviceConnection = start_file_service(device);
    struct afc_connection* fileConnection;
    AFCConnectionOpen(serviceConnection, 0, &fileConnection);
    
    char *fileDir = command.file_path;
    char *target_dir = command.destination_path;
    afc_file_ref file_ref;
    
    struct stat buf;
    stat(fileDir, &buf);
    
    printf("%lld", buf.st_size);
    unsigned int file_size = buf.st_size;
    printf("%u", file_size);
    FILE* pFile = fopen(fileDir, "rb");
    char* content = malloc(file_size);
    
    if (pFile)
    {
        fread(content, file_size, 1, pFile);
    }
    
    ASSERT_OR_EXIT(AFCFileRefOpen(fileConnection, target_dir, 3, &file_ref) == 0, "Error attempting to upload file: AFCFileRefOpen failed\n");
    ASSERT_OR_EXIT(AFCFileRefWrite(fileConnection, file_ref, content, file_size) == 0, "Error attempting to upload file: AFCFileRefWrite failed\n");
    fclose(pFile);
    
    ASSERT_OR_EXIT(AFCFileRefClose(fileConnection, file_ref) == 0, "Error attempting to upload file: AFCFileRefClose failed\n");
    ASSERT_OR_EXIT(AFCConnectionClose(fileConnection) == 0, "Error attempting to upload file: AFCConnectionClose failed\n");
    
    printf("%s successfully upload to %s.\n", command.file_path, command.destination_path);
}

// Device Connected

void on_device_connected(struct am_device *device)
{
    switch (command.type)
    {
        case GetUDID:
            get_udid(device);
            break;
            
        case InstallApp:
            install_app(device);
            break;
            
        case UninstallApp:
            uninstall_app(device);
            break;
            
        case ListApps:
            list_apps(device);
            break;
            
        case ListFiles:
            list_files(device);
            break;
            
        case RemoveFile:
            delete_file(device);
            break;
            
        case DownloadFile:
            download_file(device);
            break;
            
        case UploadFile:
            upload_file(device);
            break;
            
        default:
            break;
    }
    
    unregister_device_notification(0);
}

void confirm_udid(struct am_device *device)
{
    if (command.target)
    {
        char *udid = create_cstr_from_cfstring(AMDeviceCopyDeviceIdentifier(device));
        
        if (strcmp(command.target, udid) == 0)
        {
            on_device_connected(device);
        }
    }
    else
    {
        on_device_connected(device);
    }
}

void on_device_notification(struct am_device_notification_callback_info *info, int cookie)
{
    switch (info->msg)
    {
        case ADNCI_MSG_CONNECTED:
            confirm_udid(info->dev);
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

void process_args(int argc, char * params[])
{
    int i;
    
    for (i = 0; i < argc; i++)
    {
        if (strcmp(params[i], "-p") == 0)
        {
            command.app_path = params[i+1];
        }
        else if (strcmp(params[i], "-b") == 0)
        {
            command.bundle_id = params[i+1];
        }
        else if (strcmp(params[i], "-f") == 0)
        {
            command.file_path = params[i+1];
        }
        else if (strcmp(params[i], "-dest") == 0)
        {
            command.destination_path = params[i+1];
        }
        else if (strcmp(params[i], "-t") == 0)
        {
            command.target = params[i+1];
        }
        else if (strcmp(params[i], "-v") == 0)
        {
            command.print_paths = 1;
        }
    }
}

// Main Run Loop
int main(int argc, char * argv[])
{
    command.print_paths = 0;
    
    process_args(argc, argv);
    
    if (argc >= 2 && strcmp(argv[1], "get_udid") == 0)
    {
        command.type = GetUDID;
    }
    else if (argc >= 2 && strcmp(argv[1], "get_bundle_id") == 0)
    {
        get_bundle_id(command.app_path);
        exit(1);
    }
    else if (argc >= 2 && strcmp(argv[1], "install") == 0)
    {
        command.type = InstallApp;
    }
    else if(argc >= 2 && strcmp(argv[1], "uninstall") == 0)
    {
        command.type = UninstallApp;
    }
    else if(argc >= 2 && strcmp(argv[1], "list_apps") == 0)
    {
        command.type = ListApps;
    }
    else if(argc >= 2 && strcmp(argv[1], "list_files") == 0)
    {
        command.type = ListFiles;
    }
    else if(argc >= 2 && strcmp(argv[1], "remove_file") == 0)
    {
        command.type = RemoveFile;
    }
    else if(argc >= 2 && strcmp(argv[1], "download_file") == 0)
    {
        command.type = DownloadFile;
    }
    else if(argc >= 2 && strcmp(argv[1], "upload_file") == 0)
    {
        command.type = UploadFile;
    }
    else
    {
        print_usage();
        exit(1);
    }
    
    register_device_notification();
    return 1;
}

