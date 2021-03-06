//
//  RKMappingOperation.h
//  RestKit
//
//  Created by Blake Watters on 4/30/11.
//  Copyright (c) 2009-2012 RestKit. All rights reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#import "RKObjectMapping.h"
#import "RKAttributeMapping.h"

@class RKMappingOperation, RKDynamicMapping, RKConnectionDescription, RKMappingInfo;
@protocol RKMappingOperationDataSource;

/**
 Objects acting as the delegate for `RKMappingOperation` objects must adopt the `RKMappingOperationDelegate` protocol. These methods enable the delegate to be notified of events such as the application of attribute and relationship mappings during a mapping operation.
 */
@protocol RKMappingOperationDelegate  <NSObject>

///---------------------------------------
/// @name Tracking Property Mapping Events
///---------------------------------------

@optional

/**
 Tells the delegate that an attribute or relationship mapping was found for a given key path within the data being mapped.

 @param operation The object mapping operation being performed.
 @param keyPath The key path in the source object for which the mapping is to be applied.
 @param value The value that was found at the given key path in the source object representation.
 @param propertyMapping The `RKAttributeMapping` or `RKRelationshipMapping` for which the mappable value was found within the source object representation.
 */
- (void)mappingOperation:(RKMappingOperation *)operation didFindValue:(id)value forKeyPath:(NSString *)keyPath mapping:(RKPropertyMapping *)propertyMapping;

/**
 Tells the delegate that no attribute or relationships mapping was found for a given key path within the data being mapped.

 @param operation The object mapping operation being performed.
 @param keyPath The key path in the source object for which no mappable value was found.
 @param propertyMapping The `RKAttributeMapping` or `RKRelationshipMapping` for which no mappable value could be found within the source object representation.
 */
- (void)mappingOperation:(RKMappingOperation *)operation didNotFindValueForKeyPath:(NSString *)keyPath mapping:(RKPropertyMapping *)propertyMapping;

/**
 Tells the delegate that the mapping operation has set a value for a given key path with an attribute or relationship mapping.

 @param operation The object mapping operation being performed.
 @param value A new value that was set on the destination object.
 @param keyPath The key path in the destination object for which a new value has been set.
 @param propertyMapping The `RKAttributeMapping` or `RKRelationshipMapping` found for the key path.
 */
- (void)mappingOperation:(RKMappingOperation *)operation didSetValue:(id)value forKeyPath:(NSString *)keyPath usingMapping:(RKPropertyMapping *)propertyMapping;

/**
 Tells the delegate that the mapping operation has declined to set a value for a given key path because the value has not changed.

 @param operation The object mapping operation being performed.
 @param value A unchanged value for the key path in the destination object.
 @param keyPath The key path in the destination object for which a unchanged value was not set.
 @param propertyMapping The `RKAttributeMapping` or `RKRelationshipMapping` found for the key path.
 */
- (void)mappingOperation:(RKMappingOperation *)operation didNotSetUnchangedValue:(id)value forKeyPath:(NSString *)keyPath usingMapping:(RKPropertyMapping *)propertyMapping;

/**
 Tells the delegate that the mapping operation has failed due to an error.

 @param operation The object mapping operation that has failed.
 @param error An error object indicating the reason for the failure.
 */
- (void)mappingOperation:(RKMappingOperation *)operation didFailWithError:(NSError *)error;

///-----------------------------------------
/// @name Tracking Dynamic Mapping Selection
///-----------------------------------------

/**
 Tells the delegate that the mapping operation has selected a concrete object mapping with which to map the source object.

 Only sent if the receiver was initialized with an instance of `RKDynamicMapping` as the mapping.

 @param operation The mapping operation.
 @param objectMapping The concrete object mapping with which to perform the mapping.
 @param dynamicMapping The dynamic source mapping from which the object mapping was determined.
 */
- (void)mappingOperation:(RKMappingOperation *)operation didSelectObjectMapping:(RKObjectMapping *)objectMapping forDynamicMapping:(RKDynamicMapping *)dynamicMapping;

#ifdef _COREDATADEFINES_H

///----------------------------------------
/// @name Tracking Relationship Connections
///----------------------------------------

/**
 Tells the delegate that the mapping operation has connected a relationship.

 Only sent when mapping an `RKEntityMapping` object that contains connection mappings.

 @param operation The mapping operation.
 @param relationship The relationship that was connected.
 @param value The value that was connected to the relationship
 @param connection The connection object describing how the relationship was to be connected.
 */
- (void)mappingOperation:(RKMappingOperation *)operation didConnectRelationship:(NSRelationshipDescription *)relationship toValue:(id)value usingConnection:(RKConnectionDescription *)connection;

/**
 Tells the delegate that the mapping operation failed to connect a relationship.

 Only sent when mapping an `RKEntityMapping` object that contains connection mappings.

 @param operation The mapping operation.
 @param relationship The relationship that was connected.
 @param connection The connection object describing how the relationship was to be connected.
 */
- (void)mappingOperation:(RKMappingOperation *)operation didFailToConnectRelationship:(NSRelationshipDescription *)relationship usingConnection:(RKConnectionDescription *)connection;

#endif

@end

/**
 Instances of `RKMappingOperation` perform transformation between object representations according to the rules expressed in `RKObjectMapping` objects. Mapping operations provide the foundation for the RestKit object mapping engine and perform the work of inspecting the attributes and relationships of a source object and determining how to map them into new representations on a destination object.

 ## Metadata Mapping

 The mapping operation provides support for mapping for a dictionary of metadata in addition to the source object. This metadata is made available by mapping key paths nested under a specially designated parent key that cannot exist in a source representation. By convention, metadata is typically nested under sub keys to effectively namespace usage between components. The object mapping engine itself reserves the 'mapping' key for its usage. Metadata is passed down through a hierarchy of mapping operations (i.e. as relationships are traversed), making a common set of ancillary information available for mapping for by any operation executed.

 To understand how metadata works, consider the following example:

    @interface RKMetadataExample : NSObject
    @property (nonatomic, copy) NSString *name;
    @property (nonatomic, copy) NSURL *URL;
    @property (nonatomic, copy) NSDate *mappedAt;
    @end

    RKMetadataExample *example = [RKMetadataExample new];
    NSDictionary *representation = @{ @"name": @"Blake Watters" };
    NSDictionary *metadata = @{ @"URL": [NSURL URLWithString:@"http://restkit.org"] };

    RKObjectMapping *mapping = [RKObjectMapping mappingForClass:[RKMetadataExample class]];
    [mapping addAttributeMappingsFromDicitonary:@{ @"name": @"name", @"@metadata.URL": @"URL" }];
    RKMappingOperation *mappingOperation = [[RKObjectMapping alloc] initWithSourceObject:representation destinationObject:example mapping:
    NSError *error = nil;
    BOOL success = [mappingOperation execute:&error];

 Note the use of the special keyPath `@"@metadata.URL"`. The `@metadata` prefix indicates that the property is to be mapped from the metadata dictionary instead of from the source object representation. If any relationships were mapped, it would have access to this same metadata information as well.

 In addition to any metadata provided to the mapping operation via the `metadata` property, the operation itself makes the following metadata key paths available for mapping:

 1. `@metadata.mapping.collectionIndex` - An `NSNumber` object specifying the index of the current object within a collection being mapped. This key is only available if the current representation exists within a collection.
 */
@interface RKMappingOperation : NSOperation

///---------------------------------------
/// @name Initializing a Mapping Operation
///---------------------------------------

/**
 Initializes the receiver with a source object, a destination object and an object mapping with which to perform an object mapping.
 
 @param sourceObject The source object to be mapped. Cannot be `nil`.
 @param destinationObject The destination object the results are to be mapped onto. May be `nil`, in which case a new object target object will be obtained from the `dataSource`.
 @param objectOrDynamicMapping An instance of `RKObjectMapping` or `RKDynamicMapping` defining how the mapping is to be performed.
 @return The receiver, initialized with a source object, a destination object, and a mapping.
 */
- (id)initWithSourceObject:(id)sourceObject destinationObject:(id)destinationObject mapping:(RKMapping *)objectOrDynamicMapping;

///--------------------------------------
/// @name Accessing Mapping Configuration
///--------------------------------------

/**
 A dictionary of mappable elements containing simple values or nested object structures.
 */
@property (nonatomic, strong, readonly) id sourceObject;

/**
 The target object for this operation. Mappable values in the source object will be applied to the destination object using key-value coding.
 
 If initialized with a `nil` destination object, the mapping operation will attempt to find or create a destination object via the data source and will populate the value of the `destinationObject` property.
 */
@property (nonatomic, strong, readonly) id destinationObject;

/**
 The mapping defining how values contained in the source object should be transformed to the destination object via key-value coding.

 Will either be an instance of `RKObjectMapping` or `RKDynamicMapping`.
 */
@property (nonatomic, strong, readonly) RKMapping *mapping;

/**
 The concrete object mapping for the operation.

 If the value of `mapping` is an `RKObjectMapping`, returns the same value as `mapping`. If `mapping` is an `RKDynamicMapping`, then returns the concrete `RKObjectMapping` object selected for mapping `sourceObject`.
 */
@property (nonatomic, strong, readonly) RKObjectMapping *objectMapping;

/**
 A dictionary of metadata available for mapping in addition to the source object.
 */
@property (nonatomic, copy) NSDictionary *metadata;

///-------------------------------------------
/// @name Configuring Delegate and Data Source
///-------------------------------------------

/**
 The delegate to inform of interesting events during the mapping operation lifecycle.
 */
@property (nonatomic, weak) id<RKMappingOperationDelegate> delegate;

/**
 The data source is responsible for providing the mapping operation with an appropriate target object for mapping when the `destinationObject` is `nil`.

 @see `RKMappingOperationDataSource`
 */
@property (nonatomic, weak) id<RKMappingOperationDataSource> dataSource;

///--------------------------------
/// @name Accessing Mapping Details
///--------------------------------

/**
 The error, if any, that occurred during the execution of the mapping operation.
 */
@property (nonatomic, strong, readonly) NSError *error;

/**
 Returns a dictionary containing information about the mappings applied during the execution of the operation. The keys of the dictionary are key paths into the `destinationObject` for values that were mapped and the values are instances of `RKMappingDetails` that specify the object mapping and property mappings that were applied.
 
 Mapping info is aggregated for all child mapping operations executed for relationships.
 */
@property (nonatomic, readonly) RKMappingInfo *mappingInfo;

///-------------------------
/// @name Performing Mapping
///-------------------------

/**
 Process all mappable values from the mappable dictionary and assign them to the target object according to the rules expressed in the object mapping definition

 @param error A pointer to an `NSError` reference to capture any error that occurs during the mapping. May be `nil`.
 @return A Boolean value indicating if the mapping operation was successful.
 */
- (BOOL)performMapping:(NSError **)error;

@end

/**
 Specifies the concrete object mapping and collection of property mappings that were applied for a given key path during the execution of an `RKMappingOperation`.
 */
@interface RKMappingInfo : NSObject

/**
 The mapping that was applied.
 */
@property (nonatomic, strong, readonly) RKObjectMapping *objectMapping;

/**
 The dynamic mapping, if any, that was used to perform the mapping.
 */
@property (nonatomic, strong, readonly) RKDynamicMapping *dynamicMapping;

/**
 The set of property mappings that were applied from the mapping. An empty set indicates that the mapping matched the representation, but all values were unchanged and thus no properties were set.
 */
@property (nonatomic, readonly) NSSet *propertyMappings;

/**
 A dictionary whose keys are the destination key path for a mapped relationship and the value is an array of `RKMappingInfo` objects specifying the mapping details for each item within the collection.
 */
@property (nonatomic, readonly) NSDictionary *relationshipMappingInfo;

///--------------------------------------
/// @name Accessing Property by Subscript
///--------------------------------------

/**
 Retrieves the property mapping with the specified destination key path.
 
 @param key An `NSString` object specifying the destination key-path for the property that is to be retrieved.
 @return The `RKPropertyMapping` with the specified destination key path or `nil` if none was found.
 */
- (id)objectForKeyedSubscript:(id)key;

@end
