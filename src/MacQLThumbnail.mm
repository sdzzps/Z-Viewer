//Import modules of Cocoa API
#import <Foundation/Foundation.h>
#import <QuickLookThumbnailing/QuickLookThumbnailing.h>
#import <AppKit/AppKit.h>

//Import Qt data types
#include <QString>
#include <QSize>

//static method: convert string of local path into Mac OS Cocoa NSURL
static NSURL* toNSURL(const QString& path)
{
    NSString* p = path.toNSString(); //convert to NSString (only in .mm)
    return [NSURL fileURLWithPath:p]; //convert to NSURL item (Objective C)
}

//generate thumb image from given source filePath, target size, and to given outPngPath, return success/fail status
bool macGenerateThumbnailToPng(const QString& filePath,
                               const QSize& targetSize,
                               const QString& outPngPath)
{
    @autoreleasepool {
        NSURL* url = toNSURL(filePath);
        if (!url) return false;

        //C style method of generating CGSize 
        CGSize size = CGSizeMake((CGFloat)targetSize.width(), (CGFloat)targetSize.height());//cast QSize int to CGFloat

        QLThumbnailGenerator* gen = [QLThumbnailGenerator sharedGenerator]; //get pointer to system shared thumbnail generator
        QLThumbnailGenerationRequest* req =
            [[QLThumbnailGenerationRequest alloc] initWithFileAtURL:url
                                                              size:size
                                                             scale:1.0
                                                 representationTypes:QLThumbnailGenerationRequestRepresentationTypeThumbnail];

        __block NSData* pngData = nil; //create nullptr to NSData and capture into block
        dispatch_semaphore_t sem = dispatch_semaphore_create(0); //create waiting 

        [gen generateBestRepresentationForRequest:req completionHandler:
         ^(QLThumbnailRepresentation* _Nullable rep, NSError* _Nullable err) {
            if (rep && [rep isKindOfClass:[QLThumbnailRepresentation class]]) {
                NSImage* image = rep.NSImage;
                if (image) {
                    CGImageRef cg = [image CGImageForProposedRect:NULL context:nil hints:nil];
                    if (cg) {
                        NSBitmapImageRep* bitmap = [[NSBitmapImageRep alloc] initWithCGImage:cg];
                        pngData = [bitmap representationUsingType:NSBitmapImageFileTypePNG properties:@{}];
                    }
                }
            }
            dispatch_semaphore_signal(sem); //end waiting
        }];

        // 等待完成（同步化）
        dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, (int64_t)(3 * NSEC_PER_SEC))); //wait 3s for generation
        if (!pngData || [pngData length] == 0) return false; //if png data not created return failure

        NSString* out = outPngPath.toNSString();
        return [pngData writeToFile:out atomically:YES];
    }
}
